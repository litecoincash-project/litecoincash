// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_HIVETABLEMODEL_H
#define BITCOIN_QT_HIVETABLEMODEL_H

#include <qt/walletmodel.h>
#include <wallet/wallet.h>

#include <QAbstractTableModel>
#include <QStringList>
#include <QDateTime>

class CWallet;

class CBeeCreationTransactionInfoLessThan
{
public:
    CBeeCreationTransactionInfoLessThan(int nColumn, Qt::SortOrder fOrder):
        column(nColumn), order(fOrder) {}
    bool operator()(CBeeCreationTransactionInfo &left, CBeeCreationTransactionInfo &right) const;
    
private:
    int column;
    Qt::SortOrder order;
};

class HiveTableModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit HiveTableModel(const PlatformStyle *_platformStyle, CWallet *wallet, WalletModel *parent);
    ~HiveTableModel();

    enum ColumnIndex {
        Created = 0,
        Count = 1,
        Status = 2,
        EstimatedTime = 3,
        Cost = 4,
        Rewards = 5,
        NUMBER_OF_COLUMNS
    };

    void updateBCTs(bool includeDeadBees);
    void getSummaryValues(int &_immature, int &_mature, int &_dead, int &_blocksFound, CAmount &_cost, CAmount &_rewardsPaid, CAmount &_profit);

    // Stuff overridden from QAbstractTableModel
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public Q_SLOTS:
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
    static QString secondsToString(qint64 seconds);
    void addBCT(const CBeeCreationTransactionInfo &bct);

    const PlatformStyle *platformStyle;
    WalletModel *walletModel;
    QStringList columns;
    QList<CBeeCreationTransactionInfo> list;
    int sortColumn;
    Qt::SortOrder sortOrder;
    int immature, mature, dead, blocksFound;
    CAmount cost, rewardsPaid, profit;
};

#endif // BITCOIN_QT_HIVETABLEMODEL_H
