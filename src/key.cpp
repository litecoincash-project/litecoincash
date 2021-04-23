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

void CKey::MakeNewKey() {
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

bool Falcon_InitSanityCheck() {
    CKey key;
    key.MakeNewKey();
    CPubKey pubkey = key.GetPubKey();
    return key.VerifyPubKey(pubkey);
}
