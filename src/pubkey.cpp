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
