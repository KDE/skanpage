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
    QImage image(QFINDTESTDATA("images/test150.jpg"));
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
    QTemporaryFile *tempImageFile = new QTemporaryFile();
    tempImageFile->open();
    image.save(tempImageFile, "PNG");
    tempImageFile->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile), pageSize, 150, 90});
    QTemporaryFile *tempImageFile2 = new QTemporaryFile();
    tempImageFile2->open();
    image.save(tempImageFile2, "PNG");
    tempImageFile2->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile2), pageSize, 150, 90});
    QTemporaryFile *tempImageFile3 = new QTemporaryFile();
    tempImageFile3->open();
    image.save(tempImageFile3, "PNG");
    tempImageFile3->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile3), pageSize, 150, 270});
    QTemporaryFile *tempImageFile4 = new QTemporaryFile();
    tempImageFile4->open();
    image.save(tempImageFile4, "PNG");
    tempImageFile4->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile4), pageSize, 150, 180});
    QTemporaryFile *tempImageFile5 = new QTemporaryFile();
    tempImageFile5->open();
    image.save(tempImageFile5, "PNG");
    tempImageFile5->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile5), pageSize, 150, 180});
    QTemporaryFile *tempImageFile6 = new QTemporaryFile();
    tempImageFile6->open();
    image.save(tempImageFile6, "PNG");
    tempImageFile6->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile6), pageSize, 150, 0});
    QTemporaryFile *tempImageFile7 = new QTemporaryFile();
    tempImageFile7->open();
    image.save(tempImageFile7, "PNG");
    tempImageFile7->close();
    testDocument.append({std::shared_ptr<QTemporaryFile>(tempImageFile7), pageSize, 150, 0});

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
