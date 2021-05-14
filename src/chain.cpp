// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>        // LitecoinCash: Hive
#include <util.h>               // LitecoinCash: Hive
#include <rpc/blockchain.h>     // LitecoinCash: Hive 1.1
#include <validation.h>         // LitecoinCash: Hive 1.1

/**
 * CChain implementation
 */
void CChain::SetTip(CBlockIndex *pindex) {
    if (pindex == nullptr) {
        vChain.clear();
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }
}

CBlockLocator CChain::GetLocator(const CBlockIndex *pindex) const {
    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        // Stop when we have added the genesis block.
        if (pindex->nHeight == 0)
            break;
        // Exponentially larger steps back, plus the genesis block.
        int nHeight = std::max(pindex->nHeight - nStep, 0);
        if (Contains(pindex)) {
            // Use O(1) CChain index if possible.
            pindex = (*this)[nHeight];
        } else {
            // Otherwise, use O(log n) skiplist.
            pindex = pindex->GetAncestor(nHeight);
        }
        if (vHave.size() > 10)
            nStep *= 2;
    }

    return CBlockLocator(vHave);
}

const CBlockIndex *CChain::FindFork(const CBlockIndex *pindex) const {
    if (pindex == nullptr) {
        return nullptr;
    }
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());
    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;
    return pindex;
}

CBlockIndex* CChain::FindEarliestAtLeast(int64_t nTime) const
{
    std::vector<CBlockIndex*>::const_iterator lower = std::lower_bound(vChain.begin(), vChain.end(), nTime,
        [](CBlockIndex* pBlock, const int64_t& time) -> bool { return pBlock->GetBlockTimeMax() < time; });
    return (lower == vChain.end() ? nullptr : *lower);
}

/** Turn the lowest '1' bit in the binary representation of a number into a '0'. */
int static inline InvertLowestOne(int n) { return n & (n - 1); }

/** Compute what height to jump back to with the CBlockIndex::pskip pointer. */
int static inline GetSkipHeight(int height) {
    if (height < 2)
        return 0;

    // Determine which height to jump back to. Any number strictly lower than height is acceptable,
    // but the following expression seems to perform well in simulations (max 110 steps to go back
    // up to 2**18 blocks).
    return (height & 1) ? InvertLowestOne(InvertLowestOne(height - 1)) + 1 : InvertLowestOne(height);
}

const CBlockIndex* CBlockIndex::GetAncestor(int height) const
{
    if (height > nHeight || height < 0) {
        return nullptr;
    }

    const CBlockIndex* pindexWalk = this;
    int heightWalk = nHeight;
    while (heightWalk > height) {
        int heightSkip = GetSkipHeight(heightWalk);
        int heightSkipPrev = GetSkipHeight(heightWalk - 1);
        if (pindexWalk->pskip != nullptr &&
            (heightSkip == height ||
             (heightSkip > height && !(heightSkipPrev < heightSkip - 2 &&
                                       heightSkipPrev >= height)))) {
            // Only follow pskip if pprev->pskip isn't better than pskip->pprev.
            pindexWalk = pindexWalk->pskip;
            heightWalk = heightSkip;
        } else {
            assert(pindexWalk->pprev);
            pindexWalk = pindexWalk->pprev;
            heightWalk--;
        }
    }
    return pindexWalk;
}

CBlockIndex* CBlockIndex::GetAncestor(int height)
{
    return const_cast<CBlockIndex*>(static_cast<const CBlockIndex*>(this)->GetAncestor(height));
}

void CBlockIndex::BuildSkip()
{
    if (pprev)
        pskip = pprev->GetAncestor(GetSkipHeight(nHeight));
}

// LitecoinCash: Hive: Grant hive-mined blocks bonus work value - they get the work value of
// their own block plus that of the PoW block behind them
arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    const Consensus::Params& consensusParams = Params().GetConsensus();
    bool verbose = false;//LogAcceptCategory(BCLog::HIVE);

    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;

    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;

    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for an arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (bnTarget+1) + 1.
    arith_uint256 bnTargetScaled = (~bnTarget / (bnTarget + 1)) + 1;

    if (block.GetBlockHeader().IsHiveMined(consensusParams)) {
        assert(block.pprev);

        // LitecoinCash: Hive 1.1: Set bnPreviousTarget from nBits in most recent pow block, not just assuming it's one back. Note this logic is still valid for Hive 1.0 so doesn't need to be gated.
        CBlockIndex* pindexTemp = block.pprev;
        while (pindexTemp->GetBlockHeader().IsHiveMined(consensusParams)) {
            assert(pindexTemp->pprev);
            pindexTemp = pindexTemp->pprev;
        }

        arith_uint256 bnPreviousTarget;
        bnPreviousTarget.SetCompact(pindexTemp->nBits, &fNegative, &fOverflow); // LitecoinCash: Hive 1.1: Set bnPreviousTarget from nBits in most recent pow block, not just assuming it's one back
        if (fNegative || fOverflow || bnPreviousTarget == 0)
            return 0;
        bnTargetScaled += (~bnPreviousTarget / (bnPreviousTarget + 1)) + 1;

        // Hive 1.1: Enable bonus chainwork for Hive blocks
        if (IsHive11Enabled(&block, consensusParams)) {
            if (verbose) {
                LogPrintf("**** HIVE-1.1: ENABLING BONUS CHAINWORK ON HIVE BLOCK %s\n", block.GetBlockHash().ToString());
                LogPrintf("**** Initial block chainwork = %s\n", bnTargetScaled.ToString());
            }
            double hiveDiff = GetDifficulty(&block, true);                                  // Current hive diff
            if (verbose) LogPrintf("**** Hive diff = %.12f\n", hiveDiff);
            unsigned int k = floor(std::min(hiveDiff/consensusParams.maxHiveDiff, 1.0) * (consensusParams.maxK - consensusParams.minK) + consensusParams.minK);

            bnTargetScaled *= k;

            if (verbose) {
                LogPrintf("**** k = %d\n", k);
                LogPrintf("**** Final scaled chainwork =  %s\n", bnTargetScaled.ToString());
            }
        }
    // Hive 1.1: Enable bonus chainwork for PoW blocks
    } else if (IsHive11Enabled(&block, consensusParams)) {
        if (verbose) {
            LogPrintf("**** HIVE-1.1: CHECKING FOR BONUS CHAINWORK ON POW BLOCK %s\n", block.GetBlockHash().ToString());
            LogPrintf("**** Initial block chainwork = %s\n", bnTargetScaled.ToString());
        }

        // Find last hive block
        CBlockIndex *currBlock = block.pprev;
        int blocksSinceHive;
        double lastHiveDifficulty = 0;

        for (blocksSinceHive = 0; blocksSinceHive < consensusParams.maxKPow; blocksSinceHive++) {
            if (currBlock->GetBlockHeader().IsHiveMined(consensusParams)) {
                lastHiveDifficulty = GetDifficulty(currBlock, true);
                if (verbose) LogPrintf("**** Got last Hive diff = %.12f, at %s\n", lastHiveDifficulty, currBlock->GetBlockHash().ToString());
                break;
            }

            assert(currBlock->pprev);
            currBlock = currBlock->pprev;
        }

        if (verbose) LogPrintf("**** Pow blocks since last Hive block = %d\n", blocksSinceHive);

        // Apply k scaling
        unsigned int k = consensusParams.maxKPow - blocksSinceHive;
        if (lastHiveDifficulty < consensusParams.powSplit1)
            k = k >> 1;
        if (lastHiveDifficulty < consensusParams.powSplit2)
            k = k >> 1;

        if (k < 1)
            k = 1;

        bnTargetScaled *= k;

        if (verbose) {
            LogPrintf("**** k = %d\n", k);
            LogPrintf("**** Final scaled chainwork =  %s\n", bnTargetScaled.ToString());
        }
    }

    return bnTargetScaled;
}

// LitecoinCash: Hive: Use this to compute estimated hashes for GetNetworkHashPS()
// LitecoinCash: MinotaurX: Only consider the requested powType
arith_uint256 GetNumHashes(const CBlockIndex& block, POW_TYPE powType)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;

    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0 || block.GetBlockHeader().IsHiveMined(Params().GetConsensus()))
        return 0;

    // LitecoinCash: MinotaurX: skip the wrong pow type
    if (IsMinotaurXEnabled(&block, Params().GetConsensus()) && block.GetBlockHeader().GetPoWType() != powType)
        return 0;
    // LitecoinCash: MinotaurX: if you ask for minotaurx hashes before it's enabled, there aren't any!
    if (!IsMinotaurXEnabled(&block, Params().GetConsensus()) && powType == POW_TYPE_MINOTAURX) 
        return 0;
 
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for an arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (bnTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}

/** Find the last common ancestor two blocks have.
 *  Both pa and pb must be non-nullptr. */
const CBlockIndex* LastCommonAncestor(const CBlockIndex* pa, const CBlockIndex* pb) {
    if (pa->nHeight > pb->nHeight) {
        pa = pa->GetAncestor(pb->nHeight);
    } else if (pb->nHeight > pa->nHeight) {
        pb = pb->GetAncestor(pa->nHeight);
    }

    while (pa != pb && pa && pb) {
        pa = pa->pprev;
        pb = pb->pprev;
    }

    // Eventually all chain branches meet at the genesis block.
    assert(pa == pb);
    return pa;
}
