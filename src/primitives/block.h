// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>
#include <crypto/minotaurx/yespower/yespower.h>  // LitecoinCash: MinotaurX

// LitecoinCash: MinotaurX: An impossible pow hash (can't meet any target)
const uint256 HIGH_HASH = uint256S("0x0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

// LitecoinCash: MinotaurX: Default value for -powalgo argument
const std::string DEFAULT_POW_TYPE = "sha256d";

// LitecoinCash: MinotaurX: Pow type names
const std::string POW_TYPE_NAMES[] = {
    "sha256d",
    "minotaurx"
};

// LitecoinCash: MinotaurX: Pow type IDs
enum POW_TYPE {
    POW_TYPE_SHA256,
    POW_TYPE_MINOTAURX,
    //
    NUM_BLOCK_TYPES
};

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash() const;

    // LitecoinCash: MinotaurX: Hashing utils
    /*
    static uint256 MinotaurXHashArbitrary(const char* data);                                    // Hash arbitrary data with MinotaurX, using internally-managed thread-local memory for YP
    static uint256 MinotaurXHashStringWithLocal(std::string data, yespower_local_t *local);     // Hash a string with MinotaurX, using provided YP thread-local memory
    */
    static uint256 MinotaurHashArbitrary(const char* data);                                     // Hash arbitrary data with classical Minotaur
    static uint256 MinotaurHashString(std::string data);                                        // Hash a string with classical Minotaur
    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    // LitecoinCash: Hive: Check if this block is hivemined
    bool IsHiveMined(const Consensus::Params& consensusParams) const {
        return (nNonce == consensusParams.hiveNonceMarker);
    }

    // LitecoinCash: MinotaurX: Get pow type from version bits
    POW_TYPE GetPoWType() const {
        return (POW_TYPE)((nVersion >> 16) & 0xFF);
    }

    // LitecoinCash: MinotaurX: Get pow type name
    std::string GetPoWTypeName() const {
        if (nVersion >= 0x20000000)
            return POW_TYPE_NAMES[0];

        POW_TYPE pt = GetPoWType();
        if (pt >= NUM_BLOCK_TYPES)
            return "unrecognised";
        return POW_TYPE_NAMES[pt];
    }
};

class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        return block;
    }

    std::string ToString() const;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(const std::vector<uint256>& vHaveIn) : vHave(vHaveIn) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // BITCOIN_PRIMITIVES_BLOCK_H
