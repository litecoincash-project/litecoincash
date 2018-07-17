// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/test/uritests.h>

#include <qt/guiutil.h>
#include <qt/walletmodel.h>

#include <QUrl>

void URITests::uriTests()
{
    SendCoinsRecipient rv;
    QUrl uri;
    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?req-dontexist="));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?dontexist="));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?label=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.label == QString("Wikipedia Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?amount=0.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 10000);

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?amount=1.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 10010000);

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?amount=100&label=Wikipedia Example"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.amount == 1000000000LL);
    QVERIFY(rv.label == QString("Wikipedia Example"));

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseBitcoinURI("litecoincash://Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?message=Wikipedia Example Address", &rv));
    QVERIFY(rv.address == QString("Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n"));
    QVERIFY(rv.label == QString());

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?req-message=Wikipedia Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?amount=1,000&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("litecoincash:Cc5zbCCUULHAq6Uo7riehHZeELKNUqdR9n?amount=1,000.0&label=Wikipedia Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));
}
