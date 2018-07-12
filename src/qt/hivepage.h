// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_HIVEPAGE_H
#define BITCOIN_QT_HIVEPAGE_H

#include <amount.h>

#include <QWidget>
#include <memory>

class ClientModel;
class TransactionFilterProxy;
class TxViewDelegate;
class PlatformStyle;
class WalletModel;

namespace Ui {
    class HivePage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

// LitecoinCash: Hive page widget
class HivePage : public QWidget
{
    Q_OBJECT

public:
    explicit HivePage(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~HivePage();

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);

public Q_SLOTS:

Q_SIGNALS:

private:
    Ui::HivePage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;

private Q_SLOTS:
};

#endif // BITCOIN_QT_HIVEPAGE_H
