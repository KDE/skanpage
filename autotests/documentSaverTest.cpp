/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <memory>

#include <QTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QPageSize>

#include "testconfig.h"
#include "../src/SkanpageUtils.h"
#include "../src/DocumentSaver.h"

class DocumentSaverTest : public QObject
{
    Q_OBJECT
    
    SkanpageUtils::DocumentPages testDocument;

private Q_SLOTS:
    void initTestCase();
    void testPDFWriter();
};

void DocumentSaverTest::initTestCase()
{
    QImage image(QStringLiteral(TESTS_IMAGES_PATH) + QStringLiteral("/test150.jpg"));
    QPageSize pageSize = QPageSize(QPageSize::A4);
    int rotation = 0;
    for (int i = 0; i < 5; i++) {
        QTemporaryFile *tempImageFile = new QTemporaryFile();
        tempImageFile->open();
        image.save(tempImageFile, "PNG");
        tempImageFile->close();
        testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile), pageSize, 150, rotation});
        rotation += 90;
        if (rotation >= 360) {
            rotation = 0;
        }
    }
}

void DocumentSaverTest::testPDFWriter()
{
    DocumentSaver tempSaver;
    QSignalSpy spy(&tempSaver, &DocumentSaver::fileSaved);

    tempSaver.saveDocument(QUrl::fromLocalFile(QStringLiteral("temp.pdf")), testDocument);
    spy.wait();
}

QTEST_MAIN(DocumentSaverTest)

#include "documentSaverTest.moc"
