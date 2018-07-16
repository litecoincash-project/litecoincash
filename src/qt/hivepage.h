// LitecoinCash: The Hive page

#ifndef BITCOIN_QT_HIVEPAGE_H
#define BITCOIN_QT_HIVEPAGE_H

#include <qt/guiutil.h>

#include <QWidget>

class PlatformStyle;
class WalletModel;

namespace Ui {
    class HivePageForm;
}

QT_BEGIN_NAMESPACE
class QFrame;
QT_END_NAMESPACE

class HivePage : public QWidget
{
    Q_OBJECT

public:
    explicit HivePage(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~HivePage();

    void setModel(WalletModel *model);

private:
    Ui::HivePageForm *ui;
    WalletModel *model;
    const PlatformStyle *platformStyle;
};

#endif // BITCOIN_QT_HIVEPAGE_H
