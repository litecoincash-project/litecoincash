// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <wallet/wallet.h>
#include <wallet/fees.h>

#include <qt/hivedialog.h>
#include <qt/forms/ui_hivedialog.h>
#include <qt/clientmodel.h>
#include <qt/sendcoinsdialog.h>

#include <qt/addressbookpage.h>
#include <qt/addresstablemodel.h>
#include <qt/bitcoinunits.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/receiverequestdialog.h>
#include <qt/hivetablemodel.h>
#include <qt/walletmodel.h>

#include <QAction>
#include <QCursor>
#include <QMessageBox>
#include <QScrollBar>
#include <QTextDocument>

#include <util.h>
#include <validation.h>

HiveDialog::HiveDialog(const PlatformStyle *_platformStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HiveDialog),
    columnResizingFixer(0),
    model(0),
    platformStyle(_platformStyle)
{
    ui->setupUi(this);

    if (!_platformStyle->getImagesOnButtons())
        ui->createBeesButton->setIcon(QIcon());
    else
        ui->createBeesButton->setIcon(_platformStyle->SingleColorIcon(":/icons/bee"));

    beeCost = totalCost = rewardsPaid = cost = profit = 0;
    immature = mature = dead = blocksFound = 0;
    lastGlobalCheckHeight = 0;
    potentialRewards = projectedReturnPerBee = 0;

    ui->globalHiveSummaryError->hide();
}

void HiveDialog::setClientModel(ClientModel *_clientModel) {
    this->clientModel = _clientModel;

    if (_clientModel) {
        connect(_clientModel, SIGNAL(numBlocksChanged(int,QDateTime,double,bool)), this, SLOT(updateData()));
    }
}

void HiveDialog::setModel(WalletModel *_model) {
    this->model = _model;

    if(_model && _model->getOptionsModel())
    {
        _model->getHiveTableModel()->sort(HiveTableModel::Created, Qt::DescendingOrder);
        connect(_model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        updateDisplayUnit();

        QTableView* tableView = ui->currentHiveView;

        tableView->verticalHeader()->hide();
        tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tableView->setModel(_model->getHiveTableModel());
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
        tableView->setColumnWidth(HiveTableModel::Created, CREATED_COLUMN_WIDTH);        
        tableView->setColumnWidth(HiveTableModel::Count, COUNT_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::Status, STATUS_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::BlocksLeft, BLOCKS_LEFT_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::BlocksFound, BLOCKS_LEFT_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::TimeLeft, TIME_LEFT_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::Cost, COST_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::Rewards, REWARDS_COLUMN_WIDTH);
        tableView->setColumnWidth(HiveTableModel::Profit, PROFIT_COLUMN_WIDTH);

        // Last 2 columns are set by the columnResizingFixer, when the table geometry is ready.
        columnResizingFixer = new GUIUtil::TableViewLastColumnResizingFixer(tableView, PROFIT_COLUMN_WIDTH, HIVE_COL_MIN_WIDTH, this);

        // Populate initial data
        updateData(true);
    }
}

HiveDialog::~HiveDialog() {
    delete ui;
}

void HiveDialog::updateData(bool forceGlobalSummaryUpdate) {
    if(IsInitialBlockDownload() || chainActive.Height() == 0) {
        ui->globalHiveSummary->hide();
        ui->globalHiveSummaryError->show();
        return;
    }
    
    const Consensus::Params& consensusParams = Params().GetConsensus();

    if(model && model->getHiveTableModel()) {
        model->getHiveTableModel()->updateBCTs(ui->includeDeadBeesCheckbox->isChecked());
        model->getHiveTableModel()->getSummaryValues(immature, mature, dead, blocksFound, cost, rewardsPaid, profit);
        
        // Update labels
        ui->rewardsPaidLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), rewardsPaid)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->costLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), cost)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->profitLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), profit)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->matureLabel->setText(QString::number(mature));
        ui->immatureLabel->setText(QString::number(immature));
        ui->blocksFoundLabel->setText(QString::number(blocksFound));

        if(dead == 0) {
            ui->deadLabel->hide();
            ui->deadTitleLabel->hide();
            ui->deadLabelSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        } else {
            ui->deadLabel->setText(QString::number(dead));
            ui->deadLabel->show();
            ui->deadTitleLabel->show();
            ui->deadLabelSpacer->changeSize(ui->immatureLabelSpacer->geometry().width(), 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
        }
    }

    beeCost = GetBeeCost(chainActive.Tip()->nHeight, consensusParams);
    ui->beeCostLabel->setText(
        BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), beeCost)
        + " "
        + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
    );
    updateTotalCostDisplay();

    if (forceGlobalSummaryUpdate || chainActive.Tip()->nHeight == lastGlobalCheckHeight + 10) { // Don't update global summary every block
        int globalImmatureBees, globalImmatureBCTs, globalMatureBees, globalMatureBCTs;
        if (!GetNetworkHiveInfo(globalImmatureBees, globalImmatureBCTs, globalMatureBees, globalMatureBCTs, potentialRewards, consensusParams)) {
            ui->globalHiveSummary->hide();
            ui->globalHiveSummaryError->show();
        } else {
            ui->globalHiveSummaryError->hide();
            ui->globalHiveSummary->show();        
            ui->globalImmatureLabel->setText(QString::number(globalImmatureBees) + " (" + QString::number(globalImmatureBCTs) + " BCTs)");
            ui->globalMatureLabel->setText(QString::number(globalMatureBees) + " (" + QString::number(globalMatureBCTs) + " BCTs)");
        }

        projectedReturnPerBee = (globalMatureBees == 0) ? 0 : potentialRewards / globalMatureBees;

        ui->potentialRewardsLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), potentialRewards)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->projectedReturnPerBeeLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), projectedReturnPerBee)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );

        ui->localHiveWeightLabel->setText((mature == 0 || globalMatureBees == 0) ? "0" : QString::number(mature / (double)globalMatureBees, 'f', 3));

        lastGlobalCheckHeight = chainActive.Tip()->nHeight;
    }

    ui->blocksTillGlobalRefresh->setText(QString::number(10 - (chainActive.Tip()->nHeight - lastGlobalCheckHeight)));
}

void HiveDialog::updateDisplayUnit() {
    if(model && model->getOptionsModel()) {
        ui->beeCostLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), beeCost)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->rewardsPaidLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), rewardsPaid)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->costLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), cost)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->profitLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), profit)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->potentialRewardsLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), potentialRewards)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );
        ui->projectedReturnPerBeeLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), projectedReturnPerBee)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );        
    }

    updateTotalCostDisplay();
}

void HiveDialog::updateTotalCostDisplay() {    
    totalCost = beeCost * ui->beeCountSpinner->value();

    if(model && model->getOptionsModel()) {
        ui->totalCostLabel->setText(
            BitcoinUnits::format(model->getOptionsModel()->getDisplayUnit(), totalCost)
            + " "
            + BitcoinUnits::shortName(model->getOptionsModel()->getDisplayUnit())
        );

        if (totalCost > model->getBalance())
            ui->beeCountSpinner->setStyleSheet("QSpinBox{background:#FF8080;}");
        else
            ui->beeCountSpinner->setStyleSheet("QSpinBox{background:white;}");
    }
}

void HiveDialog::on_beeCountSpinner_valueChanged(int i) {
    updateTotalCostDisplay();
}

void HiveDialog::on_includeDeadBeesCheckbox_stateChanged() {
    updateData();
}

void HiveDialog::on_retryGlobalSummaryButton_clicked() {
    updateData(true);
}

void HiveDialog::on_refreshGlobalSummaryButton_clicked() {
    updateData(true);
}

void HiveDialog::on_createBeesButton_clicked() {
    if (model) {
        if (totalCost > model->getBalance()) {
            QMessageBox::critical(this, tr("Error"), tr("Insufficient balance to create bees."));
            return;
        }

        QString questionString = tr("Do you wish to proceed with bee creation?");
        questionString.append("<br />");
        questionString.append("<table style=\"text-align: left;\">");
        questionString.append("<tr><td>");
        questionString.append(tr("Bee count:"));
        questionString.append("</td><td>");
        questionString.append(QString::number(ui->beeCountSpinner->value()));
        questionString.append("</td></tr><tr><td>");
        questionString.append(tr("Total cost:"));
        questionString.append("</td><td>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(model->getOptionsModel()->getDisplayUnit(), totalCost) + "<br />+ transaction fees");
        questionString.append("</td></tr></table>");
        SendConfirmationDialog confirmationDialog(tr("Confirm bee creation"), questionString, SEND_CONFIRM_DELAY, this);
        confirmationDialog.exec();
        QMessageBox::StandardButton retval = (QMessageBox::StandardButton)confirmationDialog.result();

        if (retval != QMessageBox::Yes)
            return;

        model->createBees(ui->beeCountSpinner->value(), ui->donateCommunityFundCheckbox->isChecked(), this);
    }
}

// We override the virtual resizeEvent of the QWidget to adjust tables column
// sizes as the tables width is proportional to the dialogs width.
void HiveDialog::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    columnResizingFixer->stretchColumnWidth(HiveTableModel::Rewards);
}

void HiveDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        /*
        // press return -> submit form
        if (ui->beeCountSpinner->hasFocus()) {
            event->ignore();
            on_createBeesButton_clicked();
            return;
        }*/
    }

    this->QDialog::keyPressEvent(event);
}
