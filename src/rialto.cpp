// Copyright (c) 2024 The Litecoin Cash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <validation.h>
#include <rialto.h>
#include <key.h>
#include <pubkey.h>
#include <random.h>
#include <timedata.h>
#include <rpc/server.h>
#include <crypto/sha512.h>
#include <crypto/aes.h>
#include <crypto/hmac_sha256.h>

#include <secp256k1/include/secp256k1.h>
#include <secp256k1/include/secp256k1_ecdh.h>

#include <boost/algorithm/string.hpp> // boost::trim

#include <iomanip> // std::setw

// LitecoinCash: Rialto

// Protected queue of received messages
std::vector<RialtoQueuedMessage> receivedMessageQueue;
std::mutex receivedMessageQueueMutex;
std::condition_variable receivedMessageQueueCV;

// White pages directory mapping hashes of nicknames to pubkeys
CRialtoWhitePagesDB::CRialtoWhitePagesDB(std::string dbName, size_t nCacheSize, bool fMemory, bool fWipe)
: CDBWrapper(GetDataDir() / dbName, nCacheSize, fMemory, fWipe) {
    LogPrintf("Rialto: DB online: %s\n", dbName);

    // Dump the DB to the log
    #ifdef THIS_CODE_DISABLED
    LogPrintf("%s Entry Dump:\n", dbName);
    std::vector<std::pair<std::string, std::string>> all = GetAll();
    for (auto a : all)
        LogPrintf("  %s -> %s\n", a.first, a.second);
    #endif
}

// Light wrappers around CDBWrapper methods
bool CRialtoWhitePagesDB::GetPubKeyForNick(const std::string nick, std::string &pubKey) {
    return Read(nick, pubKey);
}

bool CRialtoWhitePagesDB::SetPubKeyForNick(const std::string nick, const std::string pubKey) {
    return Write(nick, pubKey);
}

bool CRialtoWhitePagesDB::RemoveNick(const std::string nick) {
    return Erase(nick);
}

bool CRialtoWhitePagesDB::NickExists(const std::string nick) {
    return Exists(nick);
}

// Get an entire whitepages DB
std::vector<std::pair<std::string, std::string>> CRialtoWhitePagesDB::GetAll() {
    std::vector<std::pair<std::string, std::string>> results;

    std::unique_ptr<CDBIterator> it(NewIterator());
    it->SeekToFirst();
    while(it->Valid()) {
        std::string k;
        std::string v;
        it->GetKey(k);
        it->GetValue(v);

        results.push_back(std::make_pair(k, v));

        it->Next();
    }

    return results;   
}

// Callback to warn of incorrect secp256k1 usage
static void RialtoIncorrectAPIUsageCallback(const char* str, void* data) {
    LogPrint(BCLog::RIALTO, "Rialto: WARNING: SECP256K1 INCORRECT API USAGE. str=%s\n", str);
}

// Helper: Hex-encode an integer to a std::string
template<typename T>
std::string IntToHexStr(T i) {
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(sizeof(T)*2)
         << std::hex << i;
  return stream.str();
}

// Validate that given nick is in legitimate format (doesn't check it actually exists)
bool RialtoIsValidNickFormat(const std::string nick) {
    // Too short/long?
    if (nick.length() < 3 || nick.length() > 20)
        return false;

    // Has anything other than lowercase letters and underscores?
    for (char c : nick)
        if ((c < 'a' || c > 'z') && c != '_')
            return false;

    return true;
}

// Validate that given plaintext is in legitimate format
bool RialtoIsValidPlaintext(const std::string plaintext) {
    // Only spaces?
    if (plaintext.find_first_not_of(' ') == std::string::npos)
        return false;

    // Too short/long?
    if (plaintext.length() < 1 || plaintext.length() > 160)
        return false;

    // Contains non-printable chars?
    for (char c : plaintext) {
        if (c < 32 || c > 126)
            return false;
    }
    return true;
}

// Parse a layer 3 envelope and check it for validity. Returns false and sets err if fails. Optionally sets timestamp and layer2Envelope
bool RialtoParseLayer3Envelope(const std::string ciphertext, std::string &err, std::string *layer2Envelope, uint32_t *timestamp) {
    if (ciphertext.size() < RIALTO_L3_MIN_LENGTH * 2) {         // (* 2 because it's hex encoded)
        err = "Layer 3 envelope is too short.";
        return false;
    } else if (ciphertext.size() > RIALTO_L3_MAX_LENGTH * 2) {  // Ditto
        err = "Layer 3 envelope is too long (max " + std::to_string(RIALTO_L3_MAX_LENGTH) + ", found " + std::to_string(ciphertext.size()) + ").";
        return false;
    }

    // Grab the hex-encoded fields
    uint32_t nonce = std::stoul(ciphertext.substr(0, 8), nullptr, 16);
    uint32_t t = std::stoul(ciphertext.substr(8, 8), nullptr, 16);
    std::string e = ciphertext.substr(16);

    // Check message age (+/- RIALTO_MESSAGE_TTL)
    uint32_t now = GetAdjustedTime();
    if (t < now - RIALTO_MESSAGE_TTL) {
        err = "Message timestamp is too old.";
        return false;
    } else if (t > now + RIALTO_MESSAGE_TTL) {
        err = "Message timestamp is too far in the future.";
        return false;
    }

    // Check POW
    std::string dataToHash = IntToHexStr(t) + e;
    arith_uint256 hash = arith_uint256(CBlockHeader::MinotaurHashString(dataToHash + std::to_string(nonce)).ToString());
    if (hash > RIALTO_MESSAGE_POW_TARGET) {
        err = "Message doesn't meet PoW target.";
        return false;
    }

    if (timestamp)
        *timestamp = t;
    
    if (layer2Envelope)
        *layer2Envelope = e;

    return true;
}

// Encrypt a message with Rialto ECIES. Returns false and sets err if fails.
bool RialtoEncryptMessage(const std::string nickFrom, const std::string nickTo, const std::string plaintext, std::string &ciphertext, uint32_t &timestampSent, std::string &err) {
    // Basic sanity checks
    if (!RialtoIsValidPlaintext(plaintext)) {
        err = "Plaintext is invalid; 1-160 printable characters only. Cannot contain only spaces.";
        return false;
    }

    if (nickFrom == "") {
        err = "From nick is empty.";
        return false;
    }
    
    if (RialtoNickIsLocal(nickTo)) {
        err = "You can chat to yourself in the mirror, but not here! (Destination nick is local)";
        return false;
    }
    
    if (RialtoNickIsBlocked(nickTo)) {
        err = "Destination nick is blocked.";
        return false;
    }

    // ********************************************************************************************************************************************
    // Assemble Layer 1 Envelope: plaintext[1..160] + 0x00 + hexTimeStamp[8] + senderNick[3..20] + 0x00 + recipientNick[3..20] + 0x00 + signature[65]
    // ********************************************************************************************************************************************

    // Assemble the encapulated message ready to sign
    uint32_t now = GetAdjustedTime();
    std::string nowStr = IntToHexStr(now);

    std::vector<unsigned char, secure_allocator<unsigned char>> layer1EnvelopeVec(0);

    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), plaintext.begin(), plaintext.end());
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), '\0');
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), nowStr.begin(), nowStr.end());
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), nickFrom.begin(), nickFrom.end());
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), '\0');
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), nickTo.begin(), nickTo.end());
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), '\0');
    
    // Get privkeydata for the nick we're sending as, to sign the message
    std::vector<unsigned char, secure_allocator<unsigned char>> fromPrivKeyData(32);
    if (!RialtoGetLocalPrivKeyForNick(nickFrom, fromPrivKeyData.data())) {
        err = "Can't find local privkey for sending nick.";
        return false;
    }

    // Make key from privkeydata
    CKey key;
    key.Set(fromPrivKeyData.begin(), fromPrivKeyData.end(), true);

    // And sign the hash of the encapsulated message
    const uint256 messageHash = Hash(layer1EnvelopeVec.begin(), layer1EnvelopeVec.end());
    std::vector<unsigned char> messageSig;
    if (!key.SignCompact(messageHash, messageSig)) {
        err = "Couldn't sign the message.";
        return false;
    }

    // Append the signature to the encapsulated message vector
    layer1EnvelopeVec.insert(layer1EnvelopeVec.end(), messageSig.begin(), messageSig.end());

    // Throw away the privkeydata and the key
    memory_cleanse(fromPrivKeyData.data(), 32);
    memory_cleanse((void *)key.begin(), 32);

    // ********************************************************************************************************************************************
    // Encrypt Layer 1 Envelope to give Layer 2 Envelope: IV[16] + pubkey[33] + layer1EnvelopeEncrypted[RIALTO_L1_MIN_LENGTH..RIALTO_L1_MAX_LENGTH] + padding[16] + MAC[32]
    // ********************************************************************************************************************************************

    // Retrieve destination nick's public key from White Pages
    std::string destPubKeyStr;
    if (!RialtoGetGlobalPubKeyForNick(nickTo, destPubKeyStr)) {
        err = "Can't find recipient pubkey in white pages.";
        return false;
    }
    CPubKey destPubKey(ParseHex(destPubKeyStr));    // No need to decompress with destPubKey.Decompress() -- it'll be parsed into a secp256k1_pubkey

    // Generate 16 random bytes for AES IV
    std::vector<unsigned char, secure_allocator<unsigned char>> IV(16);
    GetStrongRandBytes(IV.data(), 16);

    // Generate 32 random bytes for libsecp256k1 contextSeed
    std::vector<unsigned char, secure_allocator<unsigned char>> contextSeed(32);
    GetStrongRandBytes(contextSeed.data(), 32);

    // Init libsecp256k1 context with contextSeed (prevent side-channel leakage)
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);                                  // Create memory context
    if (!ctx) {
        err = "Couldn't create secp256k1 context.";
        return false;
    }

    secp256k1_context_set_illegal_callback(ctx, RialtoIncorrectAPIUsageCallback, NULL);
    
    if (!secp256k1_context_randomize(ctx, contextSeed.data())) {                                                // Randomise the context with contextSeed to protect against side-channel leakage - See secp256k1_context_randomize() in secp256k1.h
        err = "Couldn't randomise context.";
        secp256k1_context_destroy(ctx);
        return false;
    }
    
    // Parse destPubKey pubkey into a secp256k1_pubkey
    secp256k1_pubkey destPubKeyParsed;
    if (!secp256k1_ec_pubkey_parse(ctx, &destPubKeyParsed, destPubKey.begin(), destPubKey.size())) {
        err = "Couldn't parse the destination pubkey.";
        secp256k1_context_destroy(ctx);
        return false;
    }

    // Generate an ephemeral ECC keypair
    CKey *ephemeralKey = new CKey();
    ephemeralKey->MakeNewKey(true);                          // Make a valid compressed key
    CPubKey ephemeralPubKey = ephemeralKey->GetPubKey();     // Generated with secp256k1_ec_pubkey_serialize

    // Do ECDH (ephemeralPrivKey . destPubKey) to generate the shared secret
    std::vector<unsigned char, secure_allocator<unsigned char>> sharedSecret(32);
    if (!secp256k1_ecdh(ctx, sharedSecret.data(), &destPubKeyParsed, ephemeralKey->begin())) {
        err = "Couldn't perform ECDH to get shared secret.";
        secp256k1_context_destroy(ctx);
        return false;
    }

    // Throw away the ephemeral priv key and secp256k context
    memory_cleanse((void *)ephemeralKey->begin(), 32);
    secp256k1_context_destroy(ctx);

    // KDF: Hash the 32-byte shared secret with SHA-512
    std::vector<unsigned char, secure_allocator<unsigned char>> sharedSecretHash(64);
    CSHA512 hasher;
    hasher.Write(sharedSecret.data(), 32).Finalize(sharedSecretHash.data());
    
    // Split the hash into Kenc and Kmac: hash[0..31] are the encryption key and hash[32..63] are the MAC key
    std::vector<unsigned char, secure_allocator<unsigned char>> keyEncryption(32);
    std::vector<unsigned char, secure_allocator<unsigned char>> keyMAC(32);

    memcpy(keyEncryption.data(), sharedSecretHash.data(), 32);
    memcpy(keyMAC.data(), sharedSecretHash.data() + 32, 32);

    // Throw away the sharedSecret and the hash
    memory_cleanse(sharedSecret.data(), 32);
    memory_cleanse(sharedSecretHash.data(), 64);

    // Encrypt the plaintext with AES-256-CBC, using IV as initialization vector & keyEncryption as encryption key
    std::vector<unsigned char, secure_allocator<unsigned char>> encrypted;
    encrypted.resize(layer1EnvelopeVec.size() + AES_BLOCKSIZE);

    AES256CBCEncrypt crypter(keyEncryption.data(), IV.data(), true);
    int encryptedSize = crypter.Encrypt(
        layer1EnvelopeVec.data(),
        layer1EnvelopeVec.size(),
        encrypted.data()
    );
    encrypted.resize(encryptedSize);

    // Throw away layer1EnvelopeVec and keyEncryption
    memory_cleanse(layer1EnvelopeVec.data(), layer1EnvelopeVec.size());
    memory_cleanse(keyEncryption.data(), 32);

    // Calculate a 32 byte MAC with HMAC-SHA-256, using keyMAC as key and IV + ephemeralPubKey + encrypted as data
    CHMAC_SHA256 macer(keyMAC.data(), 32);
    macer.Write(IV.data(), 16)
         .Write(ephemeralPubKey.begin(), ephemeralPubKey.size())
         .Write(encrypted.data(), encrypted.size());

    std::vector<unsigned char, secure_allocator<unsigned char>> mac;
    mac.resize(32);
    macer.Finalize(mac.data());

    // Now assemble layer2Envelope: IV + R + cipher text + MAC as a secure alloc vector
    std::vector<unsigned char, secure_allocator<unsigned char>> layer2EnvelopeVec;
    layer2EnvelopeVec.insert(layer2EnvelopeVec.end(), IV.begin(), IV.end());
    layer2EnvelopeVec.insert(layer2EnvelopeVec.end(), ephemeralPubKey.begin(), ephemeralPubKey.end());
    layer2EnvelopeVec.insert(layer2EnvelopeVec.end(), encrypted.begin(), encrypted.end());
    layer2EnvelopeVec.insert(layer2EnvelopeVec.end(), mac.begin(), mac.end());

    // ********************************************************************************************************************************************
    // Perform PoW and assemble Layer 3 Envelope: hexNonce[8] + hexTimestamp[8] + layer2Envelope[RIALTO_L2_MIN_LENGTH..RIALTO_L2_MAX_LENGTH]
    // ********************************************************************************************************************************************

    uint32_t nonce = 0;
    std::string dataToHash = nowStr + HexStr(layer2EnvelopeVec.begin(), layer2EnvelopeVec.end());

    // LitecoinCash: Rialto: Note that this is called in an RPC thread, so there's no need to worry about blocking the main thread
    // or spinning up a new thread to do the PoW. We can just do it here.

    while (true) {  // with a promise we'll break or return!
        arith_uint256 currentHash = arith_uint256(CBlockHeader::MinotaurHashString(dataToHash + std::to_string(nonce)).ToString());
        if (currentHash <= RIALTO_MESSAGE_POW_TARGET)
            break;

        nonce++;
        if (nonce == 0) {
            err = "PoW Nonce overflow.";
            return false;
        }
    }

    // We're done, assemble the final envelope
    ciphertext = IntToHexStr(nonce) + nowStr + HexStr(layer2EnvelopeVec.begin(), layer2EnvelopeVec.end());
    timestampSent = now;
    return true;
}

// Decrypt a message with Rialto ECIES. Returns false and sets err if the message is invalid or not for us.
bool RialtoDecryptMessage(const std::string layer3Envelope, std::string &err) { 
    // ********************************************************************************************************************************************
    // Parse and validate Layer 3 Envelope: hexNonce[8] + hexTimestamp[8] + layer2Envelope[RIALTO_L2_MIN_LENGTH..RIALTO_L2_MAX_LENGTH]
    // ********************************************************************************************************************************************

    std::string layer2Envelope;
    uint32_t layer3timestamp;
    if (!RialtoParseLayer3Envelope(layer3Envelope, err, &layer2Envelope, &layer3timestamp))
        return false;

    // ********************************************************************************************************************************************
    // Parse Layer 2 Envelope: IV[16] + pubkey[33] + encryptedLayer1Envelope[RIALTO_L2_MIN_LENGTH..RIALTO_L1_MAX_LENGTH] + padding[16] + MAC[32]
    // ********************************************************************************************************************************************

    // Parse the level 2 envelope into IV, ephemeralPubKey, encrypted and mac
    if (layer2Envelope.size() < RIALTO_L2_MIN_LENGTH * 2) {         // (* 2 because it's hex encoded)
        err = "Layer 2 envelope is too short.";
        return false;
    } else if (layer2Envelope.size() > RIALTO_L2_MAX_LENGTH * 2) {
        err = "Layer 2 envelope is too long.";
        return false;
    }

    std::vector<unsigned char> IV = ParseHex(layer2Envelope.substr(0, 32));
    std::vector<unsigned char> ephemeralPubKey = ParseHex(layer2Envelope.substr(32, 66));
    std::vector<unsigned char> encrypted = ParseHex(layer2Envelope.substr(98, layer2Envelope.size() - 98 - 64));
    std::vector<unsigned char> mac = ParseHex(layer2Envelope.substr(layer2Envelope.size() - 64, 64));
    
    // ********************************************************************************************************************************************
    // Decrypt Layer 1 Envelope: plaintext[1..160] + 0x00 + timeStamp[4] + senderNick[3..20] + 0x00 + recipientNick[3..20] + 0x00 + signature[65]
    // ********************************************************************************************************************************************

    if (encrypted.size() % AES_BLOCKSIZE != 0) {
        err = "Encrypted data is not a multiple of AES_BLOCKSIZE bytes.";
        return false;
    }

    // Init libsecp256k1 context
    secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);                                  // Create memory context
    if (!ctx) {
        err = "Couldn't create secp256k1 context.";
        return false;
    }
    secp256k1_context_set_illegal_callback(ctx, RialtoIncorrectAPIUsageCallback, NULL);

    // Parse the extracted pubkey
    secp256k1_pubkey ephemeralPubKeyParsed;
    if (!secp256k1_ec_pubkey_parse(ctx, &ephemeralPubKeyParsed, ephemeralPubKey.data(), ephemeralPubKey.size())) {
        err = "Couldn't parse the ephemeral pubkey.";
        secp256k1_context_destroy(ctx);
        return false;
    }

    // Iterate over local nicks, trying to decrypt with each private key
    std::vector<std::pair<std::string, std::string>> nicks = RialtoGetAllLocal();
    for (auto n : nicks) {
        std::vector<unsigned char, secure_allocator<unsigned char>> privKeyData(32);
        if (!RialtoGetLocalPrivKeyForNick(n.first, privKeyData.data())) {
            LogPrint(BCLog::RIALTO, "Error: Can't find local privkey for nick %s. IS THE WALLET LOCKED?\n", n.first);
            continue;   // Optimistically keep trying
        }

        // Generate 32 random bytes for libsecp256k1 contextSeed
        std::vector<unsigned char, secure_allocator<unsigned char>> contextSeed(32);
        GetStrongRandBytes(contextSeed.data(), 32);

        // Randomise libsecp256k1 context with contextSeed (prevent side-channel leakage)
        if (!secp256k1_context_randomize(ctx, contextSeed.data())) {                                                // Randomise the context with contextSeed to protect against side-channel leakage - See secp256k1_context_randomize() in secp256k1.h
            LogPrint(BCLog::RIALTO, "Error: Couldn't randomise context when trying as %s\n", n.first);
            memory_cleanse(privKeyData.data(), 32);     // Throw away the privkeydata
            continue;   // Continue anyway
        }

        // Do ECDH (privKey . ephemeralPubKey) to generate the shared secret
        std::vector<unsigned char, secure_allocator<unsigned char>> sharedSecret(32);
        if (!secp256k1_ecdh(ctx, sharedSecret.data(), &ephemeralPubKeyParsed, privKeyData.data())) {
            LogPrint(BCLog::RIALTO, "Error: Couldn't perform ECDH to get shared secret when trying as %s\n", n.first);
            memory_cleanse(privKeyData.data(), 32);     // Throw away the privkeydata
            memory_cleanse(sharedSecret.data(), 32);    // Throw away the sharedSecret
            continue;   // Continue anyway
        }
        
        // Throw away the privkeydata
        memory_cleanse(privKeyData.data(), 32);

        // KDF: Hash the 32-byte shared secret with SHA-512
        std::vector<unsigned char, secure_allocator<unsigned char>> sharedSecretHash(64);
        CSHA512 hasher;
        hasher.Write(sharedSecret.data(), 32).Finalize(sharedSecretHash.data());

        // Split the hash into Kenc and Kmac: hash[0..31] are the encryption key and hash[32..63] are the MAC key
        std::vector<unsigned char, secure_allocator<unsigned char>> keyEncryption(32);
        std::vector<unsigned char, secure_allocator<unsigned char>> keyMAC(32);

        memcpy(keyEncryption.data(), sharedSecretHash.data(), 32);
        memcpy(keyMAC.data(), sharedSecretHash.data() + 32, 32);

        // Throw away the sharedSecret and the hash
        memory_cleanse(sharedSecret.data(), 32);
        memory_cleanse(sharedSecretHash.data(), 64);

        // Compute MAC with HMAC-SHA-256
        CHMAC_SHA256 macer(keyMAC.data(), 32);
        macer.Write(IV.data(), 16)
             .Write(ephemeralPubKey.data(), ephemeralPubKey.size())
             .Write(encrypted.data(), encrypted.size());

        std::vector<unsigned char, secure_allocator<unsigned char>> macCalc;
        macCalc.resize(32);
        macer.Finalize(macCalc.data());

        // Throw away keyMAC
        memory_cleanse(keyMAC.data(), 32);

        // Check against MAC from message. If they don't match, message is not for this nick and decryption would fail
        if (memcmp(mac.data(), macCalc.data(), 32) != 0)
            continue;
        
        // Decrypt the Layer 1 Envelope with AES-256-CBC, using IV as initialization vector & keyEncryption as decryption key
        std::vector<unsigned char, secure_allocator<unsigned char>> layer1EnvelopeVec;
        layer1EnvelopeVec.resize(RIALTO_L1_MAX_LENGTH);
        AES256CBCDecrypt dec(keyEncryption.data(), IV.data(), true);
        int size = dec.Decrypt(encrypted.data(), encrypted.size(), layer1EnvelopeVec.data());
        layer1EnvelopeVec.resize(size);

        // Throw away keyEncryption
        memory_cleanse(keyEncryption.data(), 32);
    
        // ********************************************************************************************************************************************
        // Parse and validate Layer 1 Envelope: plaintext[1..160] + 0x00 + hexTimeStamp[8] + senderNick[3..20] + 0x00 + recipientNick[3..20] + 0x00 + signature[65]
        // ********************************************************************************************************************************************

        // Check size
        if (layer1EnvelopeVec.size() < RIALTO_L1_MIN_LENGTH) {
            err = "Layer 1 envelope is too short.";
            secp256k1_context_destroy(ctx);
            return false;
        } else if (layer1EnvelopeVec.size() > RIALTO_L1_MAX_LENGTH) {
            err = "Layer 1 envelope is too long.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Find 3 nulls in layer1EnvelopeVec
        size_t firstNull = 0, secondNull = 0, thirdNull = 0;
        for (size_t i = 0; i < layer1EnvelopeVec.size(); i++) {
            if (layer1EnvelopeVec[i] == '\0') {
                if (firstNull == 0)
                    firstNull = i;
                else if (secondNull == 0)
                    secondNull = i;
                else if (thirdNull == 0) {
                    thirdNull = i;
                    break;
                }
            }
        }

        // If any nulls are missing, fail
        if (firstNull == 0 || secondNull == 0 || thirdNull == 0) {
            err = "Nulls missing in layer1EnvelopeVec.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Extract the L1 fields. They're all unconfirmed until we validate the sig.
        std::string unconfirmedPlaintext = std::string(layer1EnvelopeVec.begin(), layer1EnvelopeVec.begin() + firstNull);
        std::string layer1timestampStr = std::string(layer1EnvelopeVec.begin() + firstNull + 1, layer1EnvelopeVec.begin() + firstNull + 9);
        std::string unconfirmedSenderNick = std::string(layer1EnvelopeVec.begin() + firstNull + 9, layer1EnvelopeVec.begin() + secondNull);
        std::string unconfirmedDestinationNick = std::string(layer1EnvelopeVec.begin() + secondNull + 1, layer1EnvelopeVec.begin() + thirdNull);
        std::vector<unsigned char> messageSig(layer1EnvelopeVec.begin() + thirdNull + 1, layer1EnvelopeVec.end());

        // Grab the encapsulated message without the sig too, to hash and verify the sig
        std::vector<unsigned char> encapsulatedMessage(layer1EnvelopeVec.begin(), layer1EnvelopeVec.begin() + thirdNull + 1);

        // Check timestamp matches the layer 3 envelope
        // (Prevent an observer repackaging a message with different timestamp and PoW to conduct a replay attack)
        uint32_t layer1timestamp = std::stoul(layer1timestampStr, nullptr, 16);    
        if (layer1timestamp != layer3timestamp) {
            err = "Layer 1 / Layer 3 Envelope timestamp mismatch.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Check both nicks look plausible
        if (!RialtoIsValidNickFormat(unconfirmedDestinationNick)) {
            err = "Invalid destination nick format. Shenanigans!";
            secp256k1_context_destroy(ctx);
            return false;
        }
        if (!RialtoIsValidNickFormat(unconfirmedSenderNick)) {
            err = "Invalid sender nick. Shenanigans!";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Check for a repackaged L1 replay attack
        // (Prevent a user repurposing a message they have received, to send it to another user while impersonating the original sender)
        if (unconfirmedDestinationNick != n.first) {
            err = "Destination nick doesn't match the nick we're trying to decrypt as. Possible repackaged-L1 replay attack. Shenanigans!";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Check the local blacklist
        if(RialtoNickIsBlocked(unconfirmedSenderNick)) {
            err = "Sender nick is blocked.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Check the plaintext is valid
        if (!RialtoIsValidPlaintext(unconfirmedPlaintext)) {
            err = "Invalid plaintext.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // Check the message sig
        std::string whitePagesPubKey;
        if (!RialtoGetGlobalPubKeyForNick(unconfirmedSenderNick, whitePagesPubKey)) {
            err = "Can't find pubkey for sending nick in White Pages.";
            secp256k1_context_destroy(ctx);
            return false;
        }
        std::vector<unsigned char> whitePagesPubKeyVec = ParseHex(whitePagesPubKey);

        const uint256 messageHash = Hash(encapsulatedMessage.begin(), encapsulatedMessage.end());
        CPubKey sigPubKey;
        if (!sigPubKey.RecoverCompact(messageHash, messageSig)) {
            err = "Strange format. Couldn't recover a pubkey from the message sig.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        if (memcmp(sigPubKey.begin(), whitePagesPubKeyVec.data(), 33) != 0) {
            err = "Forgery. Pubkey from sig doesn't match pubkey from white pages.";
            secp256k1_context_destroy(ctx);
            return false;
        }

        // It's legit \o/ Pass the timestamp, plaintext, sender and destination nicks out.
        RialtoQueuedMessage qm;
        qm.fromNick = std::vector<unsigned char, secure_allocator<unsigned char>>(unconfirmedSenderNick.begin(), unconfirmedSenderNick.end());
        qm.toNick = std::vector<unsigned char, secure_allocator<unsigned char>>(unconfirmedDestinationNick.begin(), unconfirmedDestinationNick.end());
        qm.message = std::vector<unsigned char, secure_allocator<unsigned char>>(unconfirmedPlaintext.begin(), unconfirmedPlaintext.end());
        qm.timestamp = layer1timestamp;

        // Get the lock, and add to the received message queue
        std::lock_guard<std::mutex> lock(receivedMessageQueueMutex); 
        receivedMessageQueue.push_back(qm);
        receivedMessageQueueCV.notify_one();

        secp256k1_context_destroy(ctx);
        return true;
    }

    err = "Not for us.";
    secp256k1_context_destroy(ctx);
    return false;
}

// Consume the received message queue (blocks until longpoll timeout or messages arrive)
std::vector<RialtoQueuedMessage> RialtoGetQueuedMessages() {
    std::unique_lock<std::mutex> lock(receivedMessageQueueMutex);

    // 10 second timeout on longpoll
    receivedMessageQueueCV.wait_for(lock, std::chrono::milliseconds(10000), []{ 
        return !receivedMessageQueue.empty() || !IsRPCRunning(); 
    });

    if (receivedMessageQueue.empty()) {
        //LogPrint(BCLog::RIALTO, "Rialto: Queued messages longpoll timeout\n");
        return std::vector<RialtoQueuedMessage>();
    }

    std::vector<RialtoQueuedMessage> messages = receivedMessageQueue;
    receivedMessageQueue.clear();
    LogPrint(BCLog::RIALTO, "Rialto: Queued messages retrieved\n");

    return messages;
}