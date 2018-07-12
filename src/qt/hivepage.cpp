// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/hivepage.h>
#include <ui_interface.h>

HivePage::HivePage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent), model(0)
{
}

void HivePage::setModel(WalletModel *_model)
{
    this->model = _model;
}

