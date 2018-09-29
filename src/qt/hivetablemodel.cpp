// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hivetablemodel.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>

#include <clientversion.h>
#include <streams.h>

#include <util.h>

HiveTableModel::HiveTableModel(CWallet *wallet, WalletModel *parent) : QAbstractTableModel(parent), walletModel(parent)
{
    Q_UNUSED(wallet);

    // Set column headings
    columns << tr("Created") << tr("Bee count") << tr("Bee status") << tr("Blocks left") << tr("Mined") << tr("Estimated time left") << tr("Fee paid") << tr("Rewards earned") << tr("Profit");

    sortOrder = Qt::DescendingOrder;
    sortColumn = 0;

    rewardsPaid = cost = profit = 0;
    immature = mature = dead = blocksFound = 0; 
}

HiveTableModel::~HiveTableModel() {
    // Empty destructor
}

void HiveTableModel::updateBCTs(bool includeDeadBees) {
    if (walletModel) {
        // Clear existing
        beginResetModel();
        list.clear();
        endResetModel();

        // Load entries from wallet
        std::vector<CBeeCreationTransactionInfo> vBeeCreationTransactions;
        walletModel->getBCTs(vBeeCreationTransactions, includeDeadBees);
        beginInsertRows(QModelIndex(), 0, 0);
        immature = 0, mature = 0, dead = 0, blocksFound = 0;
        cost = rewardsPaid = profit = 0;
        for (const CBeeCreationTransactionInfo& bct : vBeeCreationTransactions) {
            if (bct.beeStatus == "mature")
                mature += bct.beeCount;
            else if (bct.beeStatus == "immature")
                immature += bct.beeCount;
            else if (bct.beeStatus == "dead")
                dead += bct.beeCount;

            blocksFound += bct.blocksFound;
            cost += bct.beeFeePaid;
            rewardsPaid += bct.rewardsPaid;
            profit += bct.profit;

            list.prepend(bct);
        }
        endInsertRows();

        // Maintain correct sorting
        sort(sortColumn, sortOrder);

        // Fire signal
        QMetaObject::invokeMethod(walletModel, "newHiveSummaryAvailable", Qt::QueuedConnection);
    }
}

void HiveTableModel::getSummaryValues(int &_immature, int &_mature, int &_dead, int &_blocksFound, CAmount &_cost, CAmount &_rewardsPaid, CAmount &_profit) {
    _immature = immature;
    _mature = mature;
    _blocksFound = blocksFound;
    _cost = cost;
    _rewardsPaid = rewardsPaid;
    _dead = dead;
    _profit = profit;
}

int HiveTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);

    return list.length();
}

int HiveTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);

    return columns.length();
}

QVariant HiveTableModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid() || index.row() >= list.length())
        return QVariant();

    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        const CBeeCreationTransactionInfo *rec = &list[index.row()];
        switch(index.column()) {
            case Created:
                return (rec->time == 0) ? "Not in chain yet" : GUIUtil::dateTimeStr(rec->time);
            case Count:
                return QString::number(rec->beeCount);
            case Status:
                {
                    QString status = QString::fromStdString(rec->beeStatus);
                    if (status == "immature")
                        return "Matures in " + QString::number(rec->blocksLeft - Params().GetConsensus().beeLifespanBlocks);
                    status[0] = status[0].toUpper();
                    return status;
                }
            case BlocksLeft:
                return QString::number(rec->blocksLeft);
            case BlocksFound:
                return QString::number(rec->blocksFound);
            case TimeLeft:
                return secondsToString(rec->blocksLeft * Params().GetConsensus().nPowTargetSpacing / 2);
            case Cost:
                return BitcoinUnits::format(walletModel->getOptionsModel()->getDisplayUnit(), rec->beeFeePaid) + " " + BitcoinUnits::shortName(this->walletModel->getOptionsModel()->getDisplayUnit());
            case Rewards:
                return BitcoinUnits::format(walletModel->getOptionsModel()->getDisplayUnit(), rec->rewardsPaid) + " " + BitcoinUnits::shortName(this->walletModel->getOptionsModel()->getDisplayUnit());
            case Profit:
                return BitcoinUnits::format(walletModel->getOptionsModel()->getDisplayUnit(), rec->profit) + " " + BitcoinUnits::shortName(this->walletModel->getOptionsModel()->getDisplayUnit());                
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == Cost || index.column() == Rewards || index.column() == Profit || index.column() == Count || index.column() == BlocksLeft || index.column() == BlocksFound)
            return (int)(Qt::AlignRight|Qt::AlignVCenter);
        else
            return (int)(Qt::AlignCenter|Qt::AlignVCenter);
    }
    return QVariant();
}

bool HiveTableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    return true;
}

QVariant HiveTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal)
        if(role == Qt::DisplayRole && section < columns.size())
            return columns[section];

    return QVariant();
}

void HiveTableModel::sort(int column, Qt::SortOrder order) {
    sortColumn = column;
    sortOrder = order;
    qSort(list.begin(), list.end(), CBeeCreationTransactionInfoLessThan(column, order));
    Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(list.size() - 1, NUMBER_OF_COLUMNS - 1, QModelIndex()));
}

bool CBeeCreationTransactionInfoLessThan::operator()(CBeeCreationTransactionInfo &left, CBeeCreationTransactionInfo &right) const {
    CBeeCreationTransactionInfo *pLeft = &left;
    CBeeCreationTransactionInfo *pRight = &right;
    if (order == Qt::DescendingOrder)
        std::swap(pLeft, pRight);

    switch(column) {
        case HiveTableModel::Count:
            return pLeft->beeCount < pRight->beeCount;
        case HiveTableModel::Status:
            return pLeft->blocksLeft < pRight->blocksLeft;
        case HiveTableModel::TimeLeft:
            return pLeft->blocksLeft < pRight->blocksLeft;
        case HiveTableModel::BlocksLeft:
            return pLeft->blocksLeft < pRight->blocksLeft;
        case HiveTableModel::BlocksFound:
            return pLeft->blocksFound < pRight->blocksFound;
        case HiveTableModel::Cost:
            return pLeft->beeFeePaid < pRight->beeFeePaid;
        case HiveTableModel::Rewards:
            return pLeft->rewardsPaid < pRight->rewardsPaid;
        case HiveTableModel::Profit:
            return pLeft->profit < pRight->profit;
        case HiveTableModel::Created:
        default:
            return pLeft->time < pRight->time;
    }
}

QString HiveTableModel::secondsToString(qint64 seconds) {
    const qint64 DAY = 86400;
    qint64 days = seconds / DAY;
    QTime t = QTime(0,0).addSecs(seconds % DAY);
    return QString("%1 days %2 hours %3 mins").arg(days).arg(t.hour()).arg(t.minute());
}
