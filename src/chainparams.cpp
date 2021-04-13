// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>
#include <base58.h> // Neon: Needed for DecodeDestination()

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "this is only for a test, the tests are good, like";
    const CScript genesisOutputScript = CScript() << ParseHex("04cc831f0e29171fb75e271691bfa7cbe8ac8e01fa5c0004fb345c5c591d174f1633484731f8e8952984fc38f082099ab85f7bec00ac0ae20cd3e9a4f6c02f83ce") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.BIP16Height = 0;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Neon: Hive: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nStartTime =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Neon: Hive 1.1: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nStartTime = 1618257080; 
        /*
         * Is the genesis block time (start) to avoid the
         * crash caused by the start time of Hive 1.0 and 1.1
         */
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Neon: Hive: Consensus Fields
        consensus.minBeeCost = 10000;                       // Minimum cost of a bee, used when no more block rewards
        consensus.beeCostFactor = 2500;                     // Bee cost is block_reward/beeCostFactor
        consensus.beeCreationAddress = "CReateNeonWorkerBeeXYs19YQ";        // Unspendable address for bee creation
        consensus.hiveCommunityAddress = "CashCFfv8CmdWo6wyMGQWtmQnaToyhgsWr";      // Community fund address
        consensus.communityContribFactor = 10;              // Optionally, donate bct_value/maxCommunityContribFactor to community fund
        consensus.beeGestationBlocks = 48*24;               // The number of blocks for a new bee to mature
        consensus.beeLifespanBlocks = 48*24*14;             // The number of blocks a bee lives for after maturation
        consensus.powLimitHive = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");  // Highest (easiest) bee hash target
        consensus.minHiveCheckBlock = 1;                    // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
        consensus.hiveTargetAdjustAggression = 30;          // Snap speed for bee hash target adjustment EMA
        consensus.hiveBlockSpacingTarget = 2;               // Target Hive block frequency (1 out of this many blocks should be Hivemined)
        consensus.hiveBlockSpacingTargetTypical = 3;        // Observed Hive block frequency (1 out of this many blocks are observed to be Hive)
        consensus.hiveBlockSpacingTargetTypical_1_1 = 2;    // Observed Hive block frequency in Hive 1.1 (1 out of this many blocks are observed to be Hive)
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks

        // Neon: Hive 1.1-related consensus fields
        consensus.minK = 2;                                 // Minimum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxK = 16;                                // Maximum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxHiveDiff = 0.006;                      // Hive difficulty at which max chainwork bonus is awarded
        consensus.maxKPow = 5;                              // Maximum chainwork scale for PoW blocks
        consensus.powSplit1 = 0.005;                        // Below this Hive difficulty threshold, PoW block chainwork bonus is halved
        consensus.powSplit2 = 0.0025;                       // Below this Hive difficulty threshold, PoW block chainwork bonus is halved again
        consensus.maxConsecutiveHiveBlocks = 2;             // Maximum hive blocks that can occur consecutively before a PoW block is required
        consensus.hiveDifficultyWindow = 36;                // How many blocks the SMA averages over in hive difficulty adjust

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");  // Neon: [NEED UPDATE LATER]

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0xd8ee19cd57348326c177134619778e82fee8b180180030158609f7d1b4285cb9"); // Neon: 1695238

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x3a;
        pchMessageStart[1] = 0xac;
        pchMessageStart[2] = 0x68;
        pchMessageStart[3] = 0x76;
        nDefaultPort = 32458;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1618257080, 2085618204, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0xd8ee19cd57348326c177134619778e82fee8b180180030158609f7d1b4285cb9"));
        assert(genesis.hashMerkleRoot == uint256S("0xf4fde5c1be3fcb865d79dfe45cd0ad00ca71c1bdcb27e088ba0025b0a85a4045"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        //vSeeds.emplace_back("seeds.litecoinca.sh");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,53);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,50);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1,176);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "neon";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                {  0, uint256S("0xd8ee19cd57348326c177134619778e82fee8b180180030158609f7d1b4285cb9")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block c3b156fd45db915f60b139f251cddc3f403686b0eb82a5edf57408599fda44b9 (height 1867668).
            1618257080, // * UNIX timestamp of last known number of transactions
            0,   // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0      // * estimated number of transactions per second after that timestamp
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.BIP16Height = 0; // always enforce BIP16
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 30; // Require 75% of last 40 blocks to activate rulechanges
        consensus.nMinerConfirmationWindow = 40;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
        Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Neon: Hive: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nStartTime =
            Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Neon: Hive 1.1: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nStartTime = 1618266252;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nTimeout =
            Consensus::BIP9Deployment::NO_TIMEOUT;

        // Neon: Hive: Consensus Fields
        consensus.minBeeCost = 10000;                       // Minimum cost of a bee, used when no more block rewards
        consensus.beeCostFactor = 2500;                     // Bee cost is block_reward/beeCostFactor
        consensus.beeCreationAddress = "tEstNetCreateNEONWorkerBeeXXXYq6T3r";        // Unspendable address for bee creation
        consensus.hiveCommunityAddress = "tCY5JWV4LYe64ivrAE2rD6P3bYxYtcoTsz";      // Community fund address
        consensus.communityContribFactor = 10;              // Optionally, donate bct_value/maxCommunityContribFactor to community fund
        consensus.beeGestationBlocks = 48*24;               // The number of blocks for a new bee to mature
        consensus.beeLifespanBlocks = 48*24*14;             // The number of blocks a bee lives for after maturation
        consensus.powLimitHive = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");  // Highest (easiest) bee hash target
        consensus.minHiveCheckBlock = 1;                    // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
        consensus.hiveTargetAdjustAggression = 30;          // Snap speed for bee hash target adjustment EMA
        consensus.hiveBlockSpacingTarget = 2;               // Target Hive block frequency (1 out of this many blocks should be Hivemined)
        consensus.hiveBlockSpacingTargetTypical = 3;        // Observed Hive block frequency (1 out of this many blocks are observed to be Hive)
        consensus.hiveBlockSpacingTargetTypical_1_1 = 2;    // Observed Hive block frequency in Hive 1.1 (1 out of this many blocks are observed to be Hive)
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks

        // Neon: Hive 1.1-related consensus fields
        consensus.minK = 2;                                 // Minimum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxK = 10;                                // Maximum chainwork scale for Hive blocks (see Hive whitepaper section 5)
        consensus.maxHiveDiff = 0.002;                      // Hive difficulty at which max chainwork bonus is awarded
        consensus.maxKPow = 5;                              // Maximum chainwork scale for PoW blocks
        consensus.powSplit1 = 0.001;                        // Below this Hive difficulty threshold, PoW block chainwork bonus is halved
        consensus.powSplit2 = 0.0005;                       // Below this Hive difficulty threshold, PoW block chainwork bonus is halved again
        consensus.maxConsecutiveHiveBlocks = 2;             // Maximum hive blocks that can occur consecutively before a PoW block is required
        consensus.hiveDifficultyWindow = 36;                // How many blocks the SMA averages over in hive difficulty adjust

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");  // Neon: [NEED UPDATE LATER]

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x7643b9a24ebe144d88cff06a385523f186493a09f170bb162cc545b28e9e4245"); // Neon: 166

        pchMessageStart[0] = 0x76;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xd0;
        pchMessageStart[3] = 0x82;
        nDefaultPort = 32456;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1618266252, 1062264, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x7643b9a24ebe144d88cff06a385523f186493a09f170bb162cc545b28e9e4245"));
        assert(genesis.hashMerkleRoot == uint256S("0xf4fde5c1be3fcb865d79dfe45cd0ad00ca71c1bdcb27e088ba0025b0a85a4045"));

        vFixedSeeds.clear();
        //vSeeds.emplace_back("testseeds.litecoinca.sh");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,127);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,58);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tneon";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            {
                {0, uint256S("0x7643b9a24ebe144d88cff06a385523f186493a09f170bb162cc545b28e9e4245")},
            }
        };

        chainTxData = ChainTxData{
            1618266252,
            0,
            0
        };
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Hive fields
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xca;
        pchMessageStart[2] = 0xba;
        pchMessageStart[3] = 0x5e;
        nDefaultPort = 49444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 0, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x530827f38f93b43ed12af0b3ad25a288dc02ed74d6d7857862df51fc56c416f9"));
        assert(genesis.hashMerkleRoot == uint256S("0x97ddfbbae6be97fd6cdf3e7ca13232a3afff2353e29badfab7f73011edd4ced9"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                //{0, uint256S("0x")}
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,123);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,58);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rneon";
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
