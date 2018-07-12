// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hivepage.h>

#include <qt/addresstablemodel.h>
#include <qt/bitcoinunits.h>
#include <qt/csvmodelwriter.h>
#include <qt/editaddressdialog.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/sendcoinsdialog.h>
#include <qt/transactiondescdialog.h>
#include <qt/transactionfilterproxy.h>
#include <qt/transactionrecord.h>
#include <qt/transactiontablemodel.h>
#include <qt/walletmodel.h>

#include <ui_interface.h>

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPoint>
#include <QScrollBar>
#include <QSignalMapper>
#include <QTableView>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

HivePage::HivePage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent), model(0)
{
}

void HivePage::setModel(WalletModel *_model)
{
    this->model = _model;
}

