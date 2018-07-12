// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_HIVEPAGE_H
#define BITCOIN_QT_HIVEPAGE_H

#include <qt/guiutil.h>

#include <QWidget>
#include <QKeyEvent>

class PlatformStyle;
class WalletModel;

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE

// LitecoinCash: Hive page
class HivePage : public QWidget
{
    Q_OBJECT

public:
    explicit HivePage(const PlatformStyle *platformStyle, QWidget *parent = 0);

    void setModel(WalletModel *model);

private:
    WalletModel *model;

private Q_SLOTS:

Q_SIGNALS:

    /**  Fired when a message should be reported to the user */
    void message(const QString &title, const QString &message, unsigned int style);

public Q_SLOTS:

};

#endif // BITCOIN_QT_HIVEPAGE_H
