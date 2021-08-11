// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <crypto/common.h>
#include <crypto/scrypt.h>
#include <chainparams.h>    // LitecoinCash: Hive

#include <crypto/minotaurx/minotaur.h>  // LitecoinCash: MinotaurX
#include <validation.h>                 // LitecoinCash: MinotaurX
#include <util.h>                       // LitecoinCash: MinotaurX

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

// LitecoinCash: MinotaurX: Hash arbitrary data, using internally-managed thread-local memory for YP
uint256 CBlockHeader::MinotaurXHashArbitrary(const char* data) {
    return Minotaur(data, data + strlen(data), true);
}

// LitecoinCash: MinotaurX: Hash a string, using provided YP thread-local memory
uint256 CBlockHeader::MinotaurXHashStringWithLocal(std::string data, yespower_local_t *local) {
    return Minotaur(data.begin(), data.end(), true, local);
}

// LitecoinCash: MinotaurX: Get pow hash based on block type and UASF activation
uint256 CBlockHeader::GetPoWHash() const
{
    // LitecoinCash: After powForkTime, the pow hash may be sha256 or MinotaurX
    if (nTime > Params().GetConsensus().powForkTime) {
        if (nVersion >= 0x20000000)                                 // Check for MinotaurX activation (Note: This is a safe check, so long as we are only considering blocks since LCC forked from LTC)
            return GetHash();                                       // MinotaurX not activated; definitely sha256

        switch (GetPoWType()) {                                     // Call appropriate hash for blockType
            case POW_TYPE_SHA256:
                return GetHash();
                break;
            case POW_TYPE_MINOTAURX:
                return Minotaur(BEGIN(nVersion), END(nNonce), true);
                break;
            default:                                                // Don't crash the client on invalid blockType, just return a bad hash
                return HIGH_HASH;
        }
    }
    
    // LCC not forked yet; still on Litecoin chain - definitely scrypt
    uint256 thash;
    scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    // LitecoinCash: Hive: Include type
    bool isHive = IsHiveMined(Params().GetConsensus());
    s << strprintf("CBlock(type=%s, hash=%s, powHash=%s, powType=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        isHive ? "hive" : "pow",
        GetHash().ToString(),
        GetPoWHash().ToString(),
        isHive ? "n/a" : GetPoWTypeName(),  // LitecoinCash: MinotaurX: Include pow type name
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
