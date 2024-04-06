// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/transaction.h>

#include <hash.h>
#include <tinyformat.h>
#include <utilstrencodings.h>
#include <pubkey.h> // LitecoinCash: Rialto

std::string COutPoint::ToString() const
{
    return strprintf("COutPoint(%s, %u)", hash.ToString().substr(0,10), n);
}

CTxIn::CTxIn(COutPoint prevoutIn, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = prevoutIn;
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

CTxIn::CTxIn(uint256 hashPrevTx, uint32_t nOut, CScript scriptSigIn, uint32_t nSequenceIn)
{
    prevout = COutPoint(hashPrevTx, nOut);
    scriptSig = scriptSigIn;
    nSequence = nSequenceIn;
}

std::string CTxIn::ToString() const
{
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull())
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    else
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    if (nSequence != SEQUENCE_FINAL)
        str += strprintf(", nSequence=%u", nSequence);
    str += ")";
    return str;
}

CTxOut::CTxOut(const CAmount& nValueIn, CScript scriptPubKeyIn)
{
    nValue = nValueIn;
    scriptPubKey = scriptPubKeyIn;
}

std::string CTxOut::ToString() const
{
    return strprintf("CTxOut(nValue=%d.%07d, scriptPubKey=%s)", nValue / COIN, nValue % COIN, HexStr(scriptPubKey).substr(0, 30));   // LitecoinCash: Updated num decimals
}

CMutableTransaction::CMutableTransaction() : nVersion(CTransaction::CURRENT_VERSION), nLockTime(0) {}
CMutableTransaction::CMutableTransaction(const CTransaction& tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime) {}

uint256 CMutableTransaction::GetHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 CTransaction::ComputeHash() const
{
    return SerializeHash(*this, SER_GETHASH, SERIALIZE_TRANSACTION_NO_WITNESS);
}

uint256 CTransaction::GetWitnessHash() const
{
    if (!HasWitness()) {
        return GetHash();
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/* For backward compatibility, the hash is initialized to 0. TODO: remove the need for this default constructor entirely. */
CTransaction::CTransaction() : vin(), vout(), nVersion(CTransaction::CURRENT_VERSION), nLockTime(0), hash() {}
CTransaction::CTransaction(const CMutableTransaction &tx) : vin(tx.vin), vout(tx.vout), nVersion(tx.nVersion), nLockTime(tx.nLockTime), hash(ComputeHash()) {}
CTransaction::CTransaction(CMutableTransaction &&tx) : vin(std::move(tx.vin)), vout(std::move(tx.vout)), nVersion(tx.nVersion), nLockTime(tx.nLockTime), hash(ComputeHash()) {}

CAmount CTransaction::GetValueOut() const
{
    CAmount nValueOut = 0;
    for (const auto& tx_out : vout) {
        nValueOut += tx_out.nValue;
        if (!MoneyRange(tx_out.nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    return nValueOut;
}

unsigned int CTransaction::GetTotalSize() const
{
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

std::string CTransaction::ToString() const
{
    std::string str;
    str += strprintf("CTransaction(hash=%s, ver=%d, vin.size=%u, vout.size=%u, nLockTime=%u)\n",
        GetHash().ToString().substr(0,10),
        nVersion,
        vin.size(),
        vout.size(),
        nLockTime);
    for (const auto& tx_in : vin)
        str += "    " + tx_in.ToString() + "\n";
    for (const auto& tx_in : vin)
        str += "    " + tx_in.scriptWitness.ToString() + "\n";
    for (const auto& tx_out : vout)
        str += "    " + tx_out.ToString() + "\n";
    return str;
}

// LitecoinCash: Hive: Check if this transaction is a Bee Creation Transaction, and if so return the total bee fee paid via beeFeePaid and honey scriptPubKey via scriptPubKeyHoney
bool CTransaction::IsBCT(const Consensus::Params& consensusParams, CScript scriptPubKeyBCF, CAmount* beeFeePaid, CScript* scriptPubKeyHoney) const {
    bool isBCT = CScript::IsBCTScript(vout[0].scriptPubKey, scriptPubKeyBCF, scriptPubKeyHoney);

    if (!isBCT)
        return false;

    // Grab fee
    if (beeFeePaid)
        *beeFeePaid = vout[0].nValue;

    return true;
}

// LitecoinCash: Rialto: Check if this transaction is a valid nick creation transaction, and optionally extract the nick and pubkey
bool CTransaction::IsNCT(const Consensus::Params& consensusParams, CScript scriptPubKeyNCF, std::string* pubKey, std::string* nickname) const {
    // Enough outputs?
    if (vout.size() < 2)
        return false;

    // vout[0] pays to nick creation address?
    if (vout[0].scriptPubKey != scriptPubKeyNCF)
        return false;

    // vout[1] burns enough?
    if (vout[1].nValue < consensusParams.nickCreationAntiDust)
        return false;

    // Check for vout[1] correct size
    if (vout[1].scriptPubKey.size() < 40)   // Minimum size for OP_RETURN <byte count> <pubKey[33]> OP_NICK_CREATE <byte count> <nick[3]>
        return false;

    // Check OP_RETURN and OP_NICK_CREATE are in the right place
    if (vout[1].scriptPubKey[0] != OP_RETURN || vout[1].scriptPubKey[35] != OP_NICK_CREATE)
        return false;

    // Grab pubkey
    CScript s(&vout[1].scriptPubKey[2], &vout[1].scriptPubKey[35]);
    std::string p = HexStr(s);

    // Check pubkey is valid
    CPubKey k(ParseHex(p));
    if (!k.IsFullyValid())
        return false;

    // Grab nickname
    std::string n = std::string(vout[1].scriptPubKey.begin() + 37, vout[1].scriptPubKey.end());

    // Too short/long?
    if (n.length() < 3 || n.length() > 20)
        return false;

    // Has anything other than lowercase letters and underscores?
    for (char c : n)
        if ((c < 'a' || c > 'z') && c != '_')
            return false;

    // vout[0] pays enough?
    CAmount registrationCost = consensusParams.nickCreationCostStandard;
    if (n.length() == 3)
        registrationCost = consensusParams.nickCreationCost3Char;
    else if (n.length() == 4)
        registrationCost = consensusParams.nickCreationCost4Char;

    if (vout[0].nValue < registrationCost - consensusParams.nickCreationAntiDust)
        return false;

    // Pass to calling code if wanted
    if (nickname)
        *nickname = n;

    if (pubKey)
        *pubKey = p;

    return true;
}
