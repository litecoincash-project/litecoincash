// Copyright (c) 2024 The Litecoin Cash Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOINCASH_RIALTO_H
#define LITECOINCASH_RIALTO_H

/*
LitecoinCash: Rialto: End-to-end encrypted chat.

Rialto chat uses the exact same type of secp256k keypairs that we trust to protect our
coins, and is implemented using only cryptographic primitives which already exist in
the Litecoin Cash codebase.

Our implementation encapsulates a message in three envelopes:

    Layer 1: Plaintext message, recipient and sender nicks, and timestamp, all signed by
             sender's private key.

    Layer 2: Layer 1 Envelope, encrypted with an ECIES hybrid scheme system using 
             AES-256-CBC encryption, SHA512 KDF, and HMAC-SHA256 MAC.

    Layer 3: Layer 2 Envelope, encapsulated with a Proof-of-Work nonce and timestamp.

For information on ECIES in general, see:

    https://wiki.bitmessage.org/index.php/Encryption
    https://en.wikipedia.org/wiki/Integrated_Encryption_Scheme
    https://digital.csic.es/bitstream/10261/32671/1/V2-I2-P7-13.pdf

It is impossible to tell from messages in flight who either the sender or recipient are.
A recipient must try each of their Rialto private keys to generate the ECDH shared secret
and check if the resulting MAC is valid in order to decrypt the Layer 1 envelope.

Only once the signature in the Layer 1 envelope is verified can the sender be determined,
and the message considered trusted.

Replication of the timestamp in the Layer 1 and Layer 3 envelopes protects against a replay
attack whereby an observer repackages a Layer 2 envelope with a new nonce and timestamp,
and sends it to the recipient.

Inclusion of the recipient nick in the Layer 1 envelope prevents a malicious user in receipt
of a valid message from reusing the signature and repacking the message to send to a different
recipient while appearing to be the original sender.

All messaging is off-chain and is facilitated by the P2P network only.

Privacy is prioritised over convenience in every design decision. This is a feature, not a bug.
However, this means that online/offline status is not available until your chat partner sends
you a message.

The use of ECDH in ECIES provides partial forward secrecy. Even if the sender's private key is
compromised, and an adversary has captured encrypted messages previously sent by the sender,
the messages cannot be decrypted. However, in the event that the recipient's private key is
compromised, captured messages sent to the recipient can be decrypted.

A White Pages system, populated by on-chain "Nick Creation Transactions", maps nicknames to
compressed pubkeys for use in generating ECDH shared secrets.

The Proof-of-Work component in the Layer 3 envelope uses our CPU-bound Minotaur hash algorithm.
It should only take a second to calculate on most hardware, and in conjunction with the cost
to register a nick in the White Pages, and the ability for users to maintain local blocklists,
aims to mitigate spam. Minotaur, rather than the more expensive MinotaurX, is used to ensure
that validating Layer 3 Envelopes is not a significant burden on nodes.

Rialto messaging benefits from the robust LCC P2P network and are propogated through the
network in the same way as blocks and transactions. A node advertises a hash of a Rialto
message that is has received or originated, and its connected peers request the message data
(Layer 3 Envelope) from the node. The P2P component of Rialto messaging uses the existing
inventory / getdata flow for efficient message propogation across the network. 

Upon receipt of a Rialto message, if the Layer 3 Envelope is valid, the message is in turn
advertised to connected peers. In addition, message decryption is attempted with all local
Rialto private keys. The enrypted message is propagated to peers even on successful decryption,
to prevent malicious connected peers deducing the recipient of a message.

Support for relaying Rialto messages is enabled by default, but can be optionally disabled.
Rialto messages are not propagated to peers that are not advertising support for Rialto
messaging, and nick creation transactions remain valid transactions even for un-upgraded nodes.

In this way, upgraded nodes can continue to peer with un-upgraded nodes and agree on network
consensus.
*/

#include <dbwrapper.h>
#include <arith_uint256.h>
#include <support/allocators/secure.h>  // For secure_allocator

#include <string>

// Minotaur PoW target for generating a valid Layer 3 Envelope
const arith_uint256 RIALTO_MESSAGE_POW_TARGET = arith_uint256("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

// Maximum age of a message in seconds before it is considered stale and not relayed.
// Also the age +/- which we WON'T accept an incoming message.
const int RIALTO_MESSAGE_TTL = 15 * 60;

// Length bounds for each embedded envelope layer
// Layer 1: plaintext[1..160] + 0x00 + hexTimeStamp[8] + senderNick[3..20] + 0x00 + recipientNick[3..20] + 0x00 + signature[65]
const int RIALTO_L1_MIN_LENGTH = 1 + 1 + 8 + 3 + 1 + 3 + 1 + 65;                // 83
const int RIALTO_L1_MAX_LENGTH = 160 + 1 + 8 + 20 + 1 + 20 + 1 + 65;            // 276

// Layer 2: IV[16] + pubkey[33] + level1[RIALTO_L1_MIN_LENGTH..RIALTO_L1_MAX_LENGTH] + MAC[32]
const int RIALTO_L2_MIN_LENGTH = 16 + 33 + RIALTO_L1_MIN_LENGTH + 32;           // 164
const int RIALTO_L2_MAX_LENGTH = 16 + 33 + RIALTO_L1_MAX_LENGTH + 32;           // 357

// Layer 3: hexNonce[8] + hexTimeStamp[8] + level2[RIALTO_L2_MIN_LENGTH..RIALTO_L2_MAX_LENGTH]
const int RIALTO_L3_MIN_LENGTH = 8 + 8 + RIALTO_L2_MIN_LENGTH;                  // 180
const int RIALTO_L3_MAX_LENGTH = 8 + 8 + RIALTO_L2_MAX_LENGTH;                  // 373

/*
Rialto White Pages; light extension of CDBWrapper.

Registered nicknames are mapped to compressed pubkeys (33 bytes: Sign byte 0x02 or 0x03, and 256 bit x coord. See BIP137.)

Three White Pages instances are created:
    - Global White Pages containing all nick/pubkey pairs,
    - Local White Pages containing only nick/pubkey pairs for which the local wallet has private keys.
    - Blocklist containing nicks blocked by this node.
*/
class CRialtoWhitePagesDB : public CDBWrapper {
public:
    CRialtoWhitePagesDB(std::string dbName, size_t nCacheSize, bool fMemory = false, bool fWipe = false);

    // Light wrappers around CDBWrapper methods
    bool GetPubKeyForNick(const std::string nick, std::string &pubKey);
    bool SetPubKeyForNick(const std::string nick, const std::string pubKey);
    bool RemoveNick(const std::string nick);
    bool NickExists(const std::string nick);

    // Get an entire whitepages DB
    std::vector<std::pair<std::string, std::string>> GetAll();
};

// A Rialto message encapsulated for network transmission
class CRialtoMessage {
private:
    std::string message;                    // A Layer 3 Envelope

public:
    CRialtoMessage(const std::string m) {
        message = m;
    }

    const uint256 GetHash() const {
        CHashWriter ss(SER_GETHASH, 0);     // Double SHA-256, as used for all other P2P inventory items
        ss << message;    
        return ss.GetHash();
    }
    
    const std::string GetMessage() const {
        return message;
    }
};

// A Rialto message that has been decrypted and is ready to be delivered with next call (or current longpoll!) to rialtogetincomingmessages
struct RialtoQueuedMessage {
    std::vector<unsigned char, secure_allocator<unsigned char>> fromNick;
    std::vector<unsigned char, secure_allocator<unsigned char>> toNick;
    std::vector<unsigned char, secure_allocator<unsigned char>> message;
    uint32_t timestamp;
};

// Protected queue of received messages
extern std::vector<RialtoQueuedMessage> receivedMessageQueue;
extern std::mutex receivedMessageQueueMutex;
extern std::condition_variable receivedMessageQueueCV;

// Validate that given plaintext is in legitimate format
bool RialtoIsValidPlaintext(const std::string plaintext);

// Validate that given nick is in legitimate format (doesn't check it actually exists)
bool RialtoIsValidNickFormat(const std::string nick);

// Parse a layer 3 envelope and check it for validity. Returns false and sets err if fails. Optionally sets timestamp and layer2Envelope
bool RialtoParseLayer3Envelope(
    const std::string ciphertext,
    std::string &err,
    std::string *layer2Envelope = NULL,
    uint32_t *timestamp = NULL);

// Encrypt a message with Rialto ECIES. Returns false and sets err if fails.
bool RialtoEncryptMessage(
    const std::string nickFrom,             // Sender's nickname (privkey must be in local wallet)
    const std::string nickTo,               // Recipient's nickname (pubkey must be in global white pages)
    const std::string plaintext,            // Plaintext message
    std::string &ciphertext,                // Encrypted message output
    uint32_t &timestampSent,                // Timestamp output (so that calling code can show accurate Sent timestamp)
    std::string &err                        // Error message output
);

// Decrypt a message with Rialto ECIES. Returns false and sets err if the message is invalid or not for us.
// Adds the decrypted message to the received message queue if it is for us.
bool RialtoDecryptMessage(
    const std::string layer3Envelope,       // Encrypted message (layer 3 envelope)
    std::string &err                        // Error message output
);

// Consume the received message queue (blocks until longpoll timeout or messages arrive)
std::vector<RialtoQueuedMessage> RialtoGetQueuedMessages();

#endif // BITCOIN_RIALTO_H
