// LitecoinCash: The Hive page

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
