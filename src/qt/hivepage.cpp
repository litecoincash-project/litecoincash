// LitecoinCash: The Hive page

#include <qt/hivepage.h>

HivePage::HivePage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent), model(0)
{
}

void HivePage::setModel(WalletModel *_model)
{
    this->model = _model;
}

