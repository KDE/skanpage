/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <memory>

#include <QPageSize>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QTest>
#include <QUrl>

#include "../src/DocumentSaver.h"
#include "../src/OCREngine.h"
#include "../src/SkanpageUtils.h"

class DocumentSaverTest : public QObject
{
    Q_OBJECT

    SkanpageUtils::DocumentPages testDocument;
    SkanpageUtils::DocumentPages testDocumentOCRSerif;
    SkanpageUtils::DocumentPages testDocumentOCRSans;

private Q_SLOTS:
    void initTestCase();
    void testPDFWriter();
    void testPDFOCRWriter();
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

    QImage imageOCRSerif(QFINDTESTDATA("images/testOCRSerif150.jpg"));
    QImage imageOCRSerifFlipped(QFINDTESTDATA("images/testOCRSerif150Flipped.jpg"));
    QImage imageOCRSerifRight(QFINDTESTDATA("images/testOCRSerif150Right.jpg"));
    QImage imageOCRSerifLeft(QFINDTESTDATA("images/testOCRSerif150Left.jpg"));
    rotation = 0;
    for (int i = 0; i < 5; i++) {
        QTemporaryFile *tempImageFile8 = new QTemporaryFile();
        tempImageFile8->open();
        imageOCRSerif.save(tempImageFile8, "PNG");
        tempImageFile8->close();
        testDocumentOCRSerif.append({std::shared_ptr<QTemporaryFile>(tempImageFile8), pageSize, 150, rotation});
        rotation += 90;
        if (rotation >= 360) {
            rotation = 0;
        }
    }
    for (int i = 0; i < 5; i++) {
        QTemporaryFile *tempImageFile9 = new QTemporaryFile();
        tempImageFile9->open();
        imageOCRSerifFlipped.save(tempImageFile9, "PNG");
        tempImageFile9->close();
        testDocumentOCRSerif.append({std::shared_ptr<QTemporaryFile>(tempImageFile9), pageSize, 150, rotation});
        rotation += 90;
        if (rotation >= 360) {
            rotation = 0;
        }
    }
    QSizeF pageSizeMM = pageSize.size(QPageSize::Millimeter);
    QPageSize pageSizeLandscape = QPageSize(QSizeF(pageSizeMM.height(), pageSizeMM.width()), QPageSize::Millimeter);
    QTemporaryFile *tempImageFileRight = new QTemporaryFile();
    tempImageFileRight->open();
    imageOCRSerifRight.save(tempImageFileRight, "PNG");
    tempImageFileRight->close();
    testDocumentOCRSerif.append({std::shared_ptr<QTemporaryFile>(tempImageFileRight), pageSizeLandscape, 150, 90});
    QTemporaryFile *tempImageFileLeft = new QTemporaryFile();
    tempImageFileLeft->open();
    imageOCRSerifLeft.save(tempImageFileLeft, "PNG");
    tempImageFileLeft->close();
    testDocumentOCRSerif.append({std::shared_ptr<QTemporaryFile>(tempImageFileLeft), pageSizeLandscape, 150, 270});

    QImage imageOCRSans(QFINDTESTDATA("images/testOCRSans150.jpg"));
    rotation = 0;
    for (int i = 0; i < 5; i++) {
        QTemporaryFile *tempImageFile10 = new QTemporaryFile();
        tempImageFile10->open();
        imageOCRSans.save(tempImageFile10, "PNG");
        tempImageFile10->close();
        testDocumentOCRSans.append({std::shared_ptr<QTemporaryFile>(tempImageFile10), pageSize, 150, rotation});
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

    tempSaver.saveDocument(QUrl::fromLocalFile(QStringLiteral("temp.pdf")), testDocument, SkanpageUtils::EntireDocument, QString());
    spy.wait();
}

void DocumentSaverTest::testPDFOCRWriter()
{
    DocumentSaver tempSaver;
    OCREngine ocrEngine;
    ocrEngine.setColor(Qt::red);
    tempSaver.setOCREngine(&ocrEngine);
    QSignalSpy spy(&tempSaver, &DocumentSaver::fileSaved);
    tempSaver.saveDocument(QUrl::fromLocalFile(QStringLiteral("tempOCRSerif.pdf")), testDocumentOCRSerif, SkanpageUtils::OCRDocument, QString());
    spy.wait();

    tempSaver.saveDocument(QUrl::fromLocalFile(QStringLiteral("tempOCRSans.pdf")), testDocumentOCRSans, SkanpageUtils::OCRDocument, QString());
    spy.wait();
}

QTEST_MAIN(DocumentSaverTest)

#include "documentSaverTest.moc"
