// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2017 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pubkey.h>

#include <falcon512/sign.h>

bool CPubKey::Verify(const uint256 &hash, const std::vector<unsigned char>& vchSig) const {
    if (!IsValid()) return false;

    int r = PQCLEAN_FALCON512_CLEAN_crypto_sign_verify(vchSig.data(), vchSig.size(), hash.begin(), 32, begin() + 1);
    if(r == 0) {
        return true;
    } else {
        printf("---- Falcon-512 Verify is fail.\n");
        return false;
    }
}

bool CPubKey::RecoverCompact(const uint256 &hash, const std::vector<unsigned char>& vchSig) {
    unsigned int mlen = vchSig.size() - (PUBLIC_KEY_SIZE - 1);
    unsigned char *pch = (unsigned char *)begin();
    memcpy(pch + 1, vchSig.data() + mlen, PUBLIC_KEY_SIZE - 1);
    pch[0] = 7;
    int r = PQCLEAN_FALCON512_CLEAN_crypto_sign_verify(vchSig.data(), mlen, hash.begin(), 32, pch + 1);
    if(r == 0) {
        return true;
    } else {
        printf("---- Falcon-512 RecoverCompact verify is fail.\n");
        return false;
    }
}

bool CPubKey::IsFullyValid() const {
    if (!IsValid())
        return false;
    else
        return true;
}

void CExtPubKey::Encode(unsigned char code[BIP32_EXTKEY_SIZE]) const {
    code[0] = nDepth;
    memcpy(code+1, vchFingerprint, 4);
    code[5] = (nChild >> 24) & 0xFF; code[6] = (nChild >> 16) & 0xFF;
    code[7] = (nChild >>  8) & 0xFF; code[8] = (nChild >>  0) & 0xFF;
    memcpy(code+9, chaincode.begin(), 32);
    assert(pubkey.size() == CPubKey::COMPRESSED_PUBLIC_KEY_SIZE);
    memcpy(code+41, pubkey.begin(), CPubKey::COMPRESSED_PUBLIC_KEY_SIZE);
}

void CExtPubKey::Decode(const unsigned char code[BIP32_EXTKEY_SIZE]) {
    nDepth = code[0];
    memcpy(vchFingerprint, code+1, 4);
    nChild = (code[5] << 24) | (code[6] << 16) | (code[7] << 8) | code[8];
    memcpy(chaincode.begin(), code+9, 32);
    pubkey.Set(code+41, code+BIP32_EXTKEY_SIZE);
}
