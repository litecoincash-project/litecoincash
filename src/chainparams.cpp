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
    const char* pszTimestamp = "NY Times 05/Oct/2011 Steve Jobs, Apple’s Visionary, Dies at 56";
    const CScript genesisOutputScript = CScript() << ParseHex("040184710fa689ad5023690c80f3a49c8f13f8d45b8c857fbcbc8bc4a8e4d3eb4b10f4d4604fa08dce601aaf0f470216fe1b51850b4acf21b179c45070ac7b03a9") << OP_CHECKSIG;
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
        consensus.BIP16Height = 218579; // 87afb798a3ad9378fcd56123c81fb31cfd9a8df4719b9774d71730c16315a092 - October 1, 2012
        consensus.BIP34Height = 710000;
        consensus.BIP34Hash = uint256S("fa09d204a83a768ed5a7c8d441fa62f2043abf420cff1226c7b4329aeb9d51cf");
        consensus.BIP65Height = 918684; // bab3041e8977e0dc3eeff63fe707b92bde1dd449d8efafb248c27c8264cc311a
        consensus.BIP66Height = 811879; // 7aceee012833fa8952f8835d8b1b3ae233cd6ab08fdb27a771d2bd7bdc491894
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048; // 75% of 8064
        consensus.nMinerConfirmationWindow = 8064; // nPowTargetTimespan / nPowTargetSpacing * 4
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1485561600; // January 28, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1485561600; // January 28, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

        // Neon: Hive: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nStartTime = 1545782400; // Dec 26, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nTimeout = 1577318400; // Dec 26, 2019

        // Neon: Hive 1.1: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nStartTime = 1568937600;  // Sept 20, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nTimeout = 1600560000;    // Sept 20, 2020

        // Neon fields
        consensus.powForkTime = 1518982404;                 // Time of PoW hash method change
        consensus.lastScryptBlock = 1371111;                // Height of last scrypt block
        consensus.powLimitSHA = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");   // Initial hash target at fork
        consensus.slowStartBlocks = 2000;                   // Scale post-fork block reward up over this many blocks
        consensus.premineAmount = 550000;                   // Premine amount (less than 1% of issued currency at fork time)
        std::vector<unsigned char> vch = ParseHex("76a914c9f3305556963e2976ccf3348b89a6cc736b6a4e88ac");
        consensus.premineOutputScript = CScript(vch.begin(), vch.end());	// Output script for premine block (CashierDaZEsyBQkuvv4c2uPZFx6m2XTgT)
        consensus.totalMoneySupplyHeight = 6215968;         // Height at which TMS is reached, do not issue rewards past this point

        // Neon: Hive: Consensus Fields
        consensus.minBeeCost = 10000;                       // Minimum cost of a bee, used when no more block rewards
        consensus.beeCostFactor = 2500;                     // Bee cost is block_reward/beeCostFactor
        consensus.beeCreationAddress = "CReateNeonWorkerBeeXYs19YQ";        // Unspendable address for bee creation
        consensus.hiveCommunityAddress = "CashCFfv8CmdWo6wyMGQWtmQnaToyhgsWr";      // Community fund address
        consensus.communityContribFactor = 10;              // Optionally, donate bct_value/maxCommunityContribFactor to community fund
        consensus.beeGestationBlocks = 48*24;               // The number of blocks for a new bee to mature
        consensus.beeLifespanBlocks = 48*24*14;             // The number of blocks a bee lives for after maturation
        consensus.powLimitHive = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");  // Highest (easiest) bee hash target
        consensus.minHiveCheckBlock = 1537566;              // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
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
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000ba12a25c1f2da751fc96");  // Neon: 1695238

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00000000000000238fc08340331e2735a64ac2baccdc3db0984ef65c08f658b2"); // Neon: 1695238

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xc7;
        pchMessageStart[1] = 0xe4;
        pchMessageStart[2] = 0xba;
        pchMessageStart[3] = 0xf8;
        nDefaultPort = 62458;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1317972665, 2084524493, 0x1e0ffff0, 1, 50 * COIN * COIN_SCALE);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x12a765e31ffd4059bada1e25190f6e98c99d9714d334efa41a195a7e7e04bfe2"));
        assert(genesis.hashMerkleRoot == uint256S("0x97ddfbbae6be97fd6cdf3e7ca13232a3afff2353e29badfab7f73011edd4ced9"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.emplace_back("seeds.litecoinca.sh");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,28);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,50);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1,176);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "lcc";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                {  1500, uint256S("0x841a2965955dd288cfa707a755d05a54e45f8bd476835ec9af4402a2b59a2967")},
                {  4032, uint256S("0x9ce90e427198fc0ef05e5905ce3503725b80e26afd35a987965fd7e3d9cf0846")},
                {  8064, uint256S("0xeb984353fc5190f210651f150c40b8a4bab9eeeff0b729fcb3987da694430d70")},
                { 16128, uint256S("0x602edf1859b7f9a6af809f1d9b0e6cb66fdc1d4d9dcd7a4bec03e12a1ccd153d")},
                { 23420, uint256S("0xd80fdf9ca81afd0bd2b2a90ac3a9fe547da58f2530ec874e978fce0b5101b507")},
                { 50000, uint256S("0x69dc37eb029b68f075a5012dcc0419c127672adb4f3a32882b2b3e71d07a20a6")},
                { 80000, uint256S("0x4fcb7c02f676a300503f49c764a89955a8f920b46a8cbecb4867182ecdb2e90a")},
                {120000, uint256S("0xbd9d26924f05f6daa7f0155f32828ec89e8e29cee9e7121b026a7a3552ac6131")},
                {161500, uint256S("0xdbe89880474f4bb4f75c227c77ba1cdc024991123b28b8418dbbf7798471ff43")},
                {179620, uint256S("0x2ad9c65c990ac00426d18e446e0fd7be2ffa69e9a7dcb28358a50b2b78b9f709")},
                {240000, uint256S("0x7140d1c4b4c2157ca217ee7636f24c9c73db39c4590c4e6eab2e3ea1555088aa")},
                {383640, uint256S("0x2b6809f094a9215bafc65eb3f110a35127a34be94b7d0590a096c3f126c6f364")},
                {409004, uint256S("0x487518d663d9f1fa08611d9395ad74d982b667fbdc0e77e9cf39b4f1355908a3")},
                {456000, uint256S("0xbf34f71cc6366cd487930d06be22f897e34ca6a40501ac7d401be32456372004")},
                {638902, uint256S("0x15238656e8ec63d28de29a8c75fcf3a5819afc953dcd9cc45cecc53baec74f38")},
                {721000, uint256S("0x198a7b4de1df9478e2463bd99d75b714eab235a2e63e741641dc8a759a9840e5")},
                {1371112, uint256S("0x00000000de1e4e93317241177b5f1d72fc151c6e76815e9b0be4961dfd309d60")},  // Neon: Premine block
                {1695238, uint256S("0x00000000000000238fc08340331e2735a64ac2baccdc3db0984ef65c08f658b2")},
                {1718000, uint256S("0x0000000000000059b656b7601a20df80912e6ab8bf83c63e221cdf460adebe7b")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block c3b156fd45db915f60b139f251cddc3f403686b0eb82a5edf57408599fda44b9 (height 1867668).
            1578837330, // * UNIX timestamp of last known number of transactions
            22680508,   // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.0170      // * estimated number of transactions per second after that timestamp
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
        consensus.BIP34Height = 125;
        consensus.BIP34Hash = uint256S("00000025a842c7a24366423a5a475a27e2d70148d238c68f657f8e8d5d28ea5f"); // Block hash at block 125
        consensus.BIP65Height = 125;
        consensus.BIP66Height = 125;
        consensus.powLimit = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 3.5 * 24 * 60 * 60; // 3.5 days
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 30; // Require 75% of last 40 blocks to activate rulechanges
        consensus.nMinerConfirmationWindow = 40;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1535587200; // August 30, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1535587200 + 31536000; // Start + 1 year

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1535587200; // August 30, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1535587200 + 31536000; // Start + 1 year

        // Neon: Hive: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nStartTime = 1535587200; // August 30, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE].nTimeout = 1535587200 + 31536000; // Start + 1 year

        // Neon: Hive 1.1: Deployment
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].bit = 9;
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nStartTime = 1565801161;  // August 14, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_HIVE_1_1].nTimeout = 1565801161 + 31536000;  // Start + 1 year

        // Neon fields
        consensus.powForkTime = 1565799588;                 // Time of PoW hash method change (block 100)
        consensus.lastScryptBlock = 100;                    // Height of last scrypt block
        consensus.powLimitSHA = uint256S("000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");   // Initial hash target at fork
        consensus.slowStartBlocks = 40;                     // Scale post-fork block reward up over this many blocks
        consensus.premineAmount = 550000;                   // Premine amount (less than 1% of issued currency at fork time)
        std::vector<unsigned char> vch = ParseHex("76a91424af51d38b740a6dc2868dfd70fc16d76901e1e088ac");
        consensus.premineOutputScript = CScript(vch.begin(), vch.end());	// Output script for premine block (tAGaQ7rk3NE7etu3bU6yAJF3KSKeDhTHry)
        consensus.totalMoneySupplyHeight = 6215968;         // Height at which TMS is reached, do not issue rewards past this point (Note, not accurate value for testnet)

        // Neon: Hive: Consensus Fields
        consensus.minBeeCost = 10000;                       // Minimum cost of a bee, used when no more block rewards
        consensus.beeCostFactor = 2500;                     // Bee cost is block_reward/beeCostFactor
        consensus.beeCreationAddress = "tEstNetCreateNEONWorkerBeeXXXYq6T3r";        // Unspendable address for bee creation
        consensus.hiveCommunityAddress = "tCY5JWV4LYe64ivrAE2rD6P3bYxYtcoTsz";      // Community fund address
        consensus.communityContribFactor = 10;              // Optionally, donate bct_value/maxCommunityContribFactor to community fund
        consensus.beeGestationBlocks = 48*24;               // The number of blocks for a new bee to mature
        consensus.beeLifespanBlocks = 48*24*14;             // The number of blocks a bee lives for after maturation
        consensus.powLimitHive = uint256S("0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");  // Highest (easiest) bee hash target
        consensus.minHiveCheckBlock = 100;                  // Don't bother checking below this height for Hive blocks (not used for consensus/validation checks, just efficiency when looking for potential BCTs)
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
        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000000000000000814ac185");  // Neon: 166

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("000000317630e88ba7eb1a03579c4102adb9d23dc8261ca4db27fa3ed1cad044"); // Neon: 166

        pchMessageStart[0] = 0xb6;
        pchMessageStart[1] = 0xf5;
        pchMessageStart[2] = 0xd3;
        pchMessageStart[3] = 0xcf;
        nDefaultPort = 62456;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1486949366, 293345, 0x1e0ffff0, 1, 50 * COIN * COIN_SCALE);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x4966625a4b2851d9fdee139e56211a0d88575f59ed816ff5e6a63deb4e3e29a0"));
        assert(genesis.hashMerkleRoot == uint256S("0x97ddfbbae6be97fd6cdf3e7ca13232a3afff2353e29badfab7f73011edd4ced9"));

        vFixedSeeds.clear();
        vSeeds.emplace_back("testseeds.litecoinca.sh");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,127);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,58);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tlcc";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            {
                {100, uint256S("9407e44ea211c125715f7a1a52f875edaaaa5129ad300e47b3ae8c24b4368721")},    // Last Scrypt block
                {125, uint256S("00000025a842c7a24366423a5a475a27e2d70148d238c68f657f8e8d5d28ea5f")},    // BIP34/BIP65/BIP66 activation
                {141, uint256S("0000000148fba811dcac0561d31910d231d1838304414e8520ab9084deb35aeb")},    // Pre Hive-1.1 activation
                {166, uint256S("000000317630e88ba7eb1a03579c4102adb9d23dc8261ca4db27fa3ed1cad044")},
            }
        };

        chainTxData = ChainTxData{  // As at 141
            1565801161,
            142,
            0.05
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

        // Neon fields
        consensus.powForkTime = 1543765622;                 // Time of PoW hash method change (block 100)
        consensus.lastScryptBlock = 200;                    // Height of last scrypt block
        consensus.powLimitSHA = uint256S("000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");   // Initial hash target at fork
        consensus.slowStartBlocks = 40;                     // Scale post-fork block reward up over this many blocks
        consensus.premineAmount = 550000;                   // Premine amount (less than 1% of issued currency at fork time)
        std::vector<unsigned char> vch = ParseHex("76a91424af51d38b740a6dc2868dfd70fc16d76901e1e088ac");
        consensus.premineOutputScript = CScript(vch.begin(), vch.end());	// Output script for premine block (tAGaQ7rk3NE7etu3bU6yAJF3KSKeDhTHry)
        consensus.totalMoneySupplyHeight = 6215968;         // Height at which TMS is reached, do not issue rewards past this point (Note, not accurate value for testnet)
        consensus.hiveNonceMarker = 192;                    // Nonce marker for hivemined blocks

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 19444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 0, 0x207fffff, 1, 50 * COIN * COIN_SCALE);
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
                {0, uint256S("530827f38f93b43ed12af0b3ad25a288dc02ed74d6d7857862df51fc56c416f9")}
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,58);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rlcc";
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
