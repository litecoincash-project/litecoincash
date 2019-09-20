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

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

uint256 CBlockHeader::GetPoWHash() const
{
    // LitecoinCash: After powForkTime, the pow hash is sha256
    if (nTime > Params().GetConsensus().powForkTime)
        return GetHash();
    
    uint256 thash;
    scrypt_1024_1_1_256(BEGIN(nVersion), BEGIN(thash));
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    // LitecoinCash: Hive: Include type
    s << strprintf("CBlock(type=%s, hash=%s, powHash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        IsHiveMined(Params().GetConsensus()) ? "hive" : "pow",
        GetHash().ToString(),
        GetPoWHash().ToString(),
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
