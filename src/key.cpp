// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key.h>

#include <arith_uint256.h>
#include <crypto/common.h>
#include <crypto/hmac_sha512.h>
#include <random.h>

#include <falcon512/sign.h>

void CKey::MakeNewKey(bool fCompressedIn) {
    unsigned char sk[PRIVATE_KEY_SIZE];
    unsigned char pk[PUB_KEY_SIZE];
    int r = PQCLEAN_FALCON512_CLEAN_crypto_sign_keypair(pk,sk);
    if(r != 0)
        printf("---- Falcon-512 Key pair gen fail.\n");

    memcpy(keydata.data(), sk, PRIVATE_KEY_SIZE);
    memcpy(pubkeydata.data(), pk, PUB_KEY_SIZE);
    fValid = true;
    fCompressed = true;
}

CPrivKey CKey::GetPrivKey() const {
    assert(fValid);
    CPrivKey privkey;
    privkey.resize(PRIVATE_KEY_SIZE);
    memcpy(privkey.data(), keydata.data(), keydata.size());
    return privkey;
}

CPubKey CKey::GetPubKey() const {
    assert(fValid);
    CPubKey pubkey;
    unsigned char* pch = (unsigned char *)pubkey.begin();
    memcpy(pch + 1, pubkeydata.data(), pubkeydata.size());
    pch[0] = 7;
    return pubkey;
}

bool CKey::Sign(const uint256 &hash, std::vector<unsigned char>& vchSig, uint32_t test_case) const {
    if (!fValid) return false;

    size_t sig_len;
    vchSig.resize(PQCLEAN_FALCON512_CLEAN_CRYPTO_BYTES_);
    int r = PQCLEAN_FALCON512_CLEAN_crypto_sign_signature(vchSig.data(), &sig_len, hash.begin(), 32, keydata.data());
    vchSig.resize(sig_len);
    if(r != 0)
        printf("---- Falcon-512 Signature fail.\n");

    return true;
}

bool CKey::VerifyPubKey(const CPubKey& pubkey) const {
    unsigned char rnd[8];
    std::string str = "Bitcoin key verification\n";
    GetRandBytes(rnd, sizeof(rnd));
    uint256 hash;
    CHash256().Write((unsigned char*)str.data(), str.size()).Write(rnd, sizeof(rnd)).Finalize(hash.begin());
    std::vector<unsigned char> vchSig;
    Sign(hash, vchSig);
    return pubkey.Verify(hash, vchSig);
}

bool CKey::SignCompact(const uint256 &hash, std::vector<unsigned char>& vchSig) const {
    if (!fValid) return false;

    size_t sig_len;
    vchSig.resize(PQCLEAN_FALCON512_CLEAN_CRYPTO_BYTES_ + pksize());
    int r = PQCLEAN_FALCON512_CLEAN_crypto_sign_signature(vchSig.data(), &sig_len, hash.begin(), 32, keydata.data());
    vchSig.resize(sig_len + pksize());
    memcpy(vchSig.data() + sig_len,pubkeydata.data(),pksize());
    if(r != 0)
        printf("---- Falcon-512 Signature fail.\n");

    return true;
}

bool CKey::Load(const CPrivKey &privkey, const CPubKey &vchPubKey, bool fSkipCheck = false) {
    memcpy((unsigned char*)begin(), privkey.data(), privkey.size());
    fCompressed = true;
    fValid = true;
    memcpy((unsigned char*)pkbegin(), vchPubKey.data()+1, pksize());

    if (fSkipCheck) return true;

    return VerifyPubKey(vchPubKey);
}

void CExtKey::SetMaster(const unsigned char *seed, unsigned int nSeedLen) {
    static const unsigned char hashkey[] = {'B','i','t','c','o','i','n',' ','s','e','e','d'};
    std::vector<unsigned char, secure_allocator<unsigned char>> vout(64);
    CHMAC_SHA512(hashkey, sizeof(hashkey)).Write(seed, nSeedLen).Finalize(vout.data());
    key.Set(vout.data(), vout.data() + 32, true);
    memcpy(chaincode.begin(), vout.data() + 32, 32);
    nDepth = 0;
    nChild = 0;
    memset(vchFingerprint, 0, sizeof(vchFingerprint));
}

CExtPubKey CExtKey::Neuter() const {
    CExtPubKey ret;
    ret.nDepth = nDepth;
    memcpy(&ret.vchFingerprint[0], &vchFingerprint[0], 4);
    ret.nChild = nChild;
    ret.pubkey = key.GetPubKey();
    ret.chaincode = chaincode;
    return ret;
}

void CExtKey::Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, chaincode.begin(), 32);
    code[41] = 0;
    assert(key.size() == 32);
    memcpy(code+42, key.begin(), 32);
}

void CExtKey::Decode(const unsigned char code[BIP32_EXTKEY_SIZE]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(chaincode.begin(), code+9, 32);
    key.Set(code+42, code+BIP32_EXTKEY_SIZE, true);
}

bool Falcon_InitSanityCheck() {
    CKey key;
    key.MakeNewKey(true);
    CPubKey pubkey = key.GetPubKey();
    return key.VerifyPubKey(pubkey);
}
