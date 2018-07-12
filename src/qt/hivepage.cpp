// LitecoinCash: The Hive page

#include <wallet/wallet.h>

#include <qt/hivepage.h>
#include <qt/forms/ui_hivepage.h>

#include <qt/platformstyle.h>
#include <qt/walletmodel.h>

HivePage::HivePage(const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HivePageForm),
    model(0),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);
}

void HivePage::setModel(WalletModel *_model)
{
    this->model = _model;
}

HivePage::~HivePage()
{
    delete ui;
}
