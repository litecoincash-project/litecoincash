// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hivetablemodel.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>

#include <clientversion.h>
#include <streams.h>

#include <util.h>

HiveTableModel::HiveTableModel(const PlatformStyle *_platformStyle, CWallet *wallet, WalletModel *parent) : platformStyle(_platformStyle), walletModel(parent), QAbstractTableModel(parent)
{
    Q_UNUSED(wallet);

    // Set column headings
    columns << tr("Created") << tr("Bee count") << tr("Bee status") << tr("Estimated time until status change") << tr("Fee paid") << tr("Rewards earned");

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

    const CBeeCreationTransactionInfo *rec = &list[index.row()];
    if(role == Qt::DisplayRole || role == Qt::EditRole) {        
        switch(index.column()) {
            case Created:
                return (rec->time == 0) ? "Not in chain yet" : GUIUtil::dateTimeStr(rec->time);
            case Count:
                return QString::number(rec->beeCount);
            case Status:
                {
                    QString status = QString::fromStdString(rec->beeStatus);
                    status[0] = status[0].toUpper();
                    return status;
                }
            case EstimatedTime:
                {
                    QString status = "";
                    if (rec->beeStatus == "immature") {
                        int blocksTillMature = rec->blocksLeft - Params().GetConsensus().beeLifespanBlocks;
                        status = "Matures in " + QString::number(blocksTillMature) + " blocks (" + secondsToString(blocksTillMature * Params().GetConsensus().nPowTargetSpacing / 2) + ")";
                    } else if (rec->beeStatus == "mature")
                        status = "Expires in " + QString::number(rec->blocksLeft) + " blocks (" + secondsToString(rec->blocksLeft * Params().GetConsensus().nPowTargetSpacing / 2) + ")";
                    return status;
                }
            case Cost:
                return BitcoinUnits::format(walletModel->getOptionsModel()->getDisplayUnit(), rec->beeFeePaid) + " " + BitcoinUnits::shortName(this->walletModel->getOptionsModel()->getDisplayUnit());
            case Rewards:
                if (rec->blocksFound == 0)
                    return "No blocks mined";
                return BitcoinUnits::format(walletModel->getOptionsModel()->getDisplayUnit(), rec->rewardsPaid)
                    + " " + BitcoinUnits::shortName(this->walletModel->getOptionsModel()->getDisplayUnit()) 
                    + " (" + QString::number(rec->blocksFound) + " blocks mined)";
        }
    }
    else if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == Cost && rec->blocksFound == 0)
            return (int)(Qt::AlignCenter|Qt::AlignVCenter);
        else if (index.column() == Cost || index.column() == Rewards || index.column() == Count)
            return (int)(Qt::AlignRight|Qt::AlignVCenter);
        else
            return (int)(Qt::AlignCenter|Qt::AlignVCenter);
    }
    else if (role == Qt::ForegroundRole)
    {
        const CBeeCreationTransactionInfo *rec = &list[index.row()];
        if (rec->beeStatus == "dead")
            return QColor(139, 0, 0);
        if (rec->beeStatus == "immature")
            return QColor(128, 70, 0);
        return QColor(27, 104, 45);
    }
    else if (role == Qt::DecorationRole)
    {
        const CBeeCreationTransactionInfo *rec = &list[index.row()];
        if (index.column() == Status) {
            QString iconStr = ":/icons/beestatus_dead";    // Dead
            if (rec->beeStatus == "mature")
                iconStr = ":/icons/beestatus_mature";
            else if (rec->beeStatus == "immature")
                iconStr = ":/icons/beestatus_immature";                
            return platformStyle->SingleColorIcon(iconStr);
        }
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
        case HiveTableModel::EstimatedTime:
            return pLeft->blocksLeft < pRight->blocksLeft;
        case HiveTableModel::Cost:
            return pLeft->beeFeePaid < pRight->beeFeePaid;
        case HiveTableModel::Rewards:
            return pLeft->rewardsPaid < pRight->rewardsPaid;
        case HiveTableModel::Created:
        default:
            return pLeft->time < pRight->time;
    }
}

QString HiveTableModel::secondsToString(qint64 seconds) {
    const qint64 DAY = 86400;
    qint64 days = seconds / DAY;
    QTime t = QTime(0,0).addSecs(seconds % DAY);
    return QString("%1 days %2 hrs %3 mins").arg(days).arg(t.hour()).arg(t.minute());
}
