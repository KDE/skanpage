/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DocumentSaver.h"

#include <QTransform>

#include <KLocalizedString>

#include "skanpage_debug.h"
#include "OCREngine.h"

DocumentSaver::DocumentSaver(QObject *parent)
    : QObject(parent)
    , m_OCREngine(nullptr)
{
}

DocumentSaver::~DocumentSaver()
{
}

void DocumentSaver::setOCREngine(OCREngine *engine)
{
    m_OCREngine = engine;
}

void DocumentSaver::saveDocument(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type, const QString &title)
{
    if (fileUrl.isEmpty()) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("No file path given."));
        return;
    }
    if (document.isEmpty()) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Nothing to save."));
        return;
    }
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Saving document to") << fileUrl;

    const QFileInfo &fileInfo = QFileInfo(fileUrl.toLocalFile());
    const QString &fileSuffix = fileInfo.suffix();

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Selected file suffix is") << fileSuffix;

    if (type == SkanpageUtils::OCRDocument) {
        m_OCREngine->InitForOCR();
        saveSearchablePDF(fileUrl, document, title);
    } else if (fileSuffix == QLatin1String("pdf") || fileSuffix.isEmpty()) {
        savePDF(fileUrl, document, type);
    } else {
        saveImage(fileInfo, document, type);
    }
}

void DocumentSaver::savePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type)
{
    QPdfWriter writer(fileUrl.toLocalFile());
    writer.setCreator(QStringLiteral("org.kde.skanpage"));
    QPainter painter;

    for (int i = 0; i < document.count(); ++i) {
        printPage(writer, painter, document.at(i), i == 0);
    }

    if (type == SkanpageUtils::EntireDocument || type == SkanpageUtils::PageSelection) {
        Q_EMIT showUserMessage(SkanpageUtils::InformationMessage, i18n("Document saved as PDF."));
    }

    if (type == SkanpageUtils::EntireDocument) {
        Q_EMIT fileSaved({fileUrl}, document);
    } else if (type == SkanpageUtils::SharingDocument) {
        Q_EMIT sharingFileSaved({fileUrl});
    }
}

void DocumentSaver::saveSearchablePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const QString &title)
{
    if (m_OCREngine == nullptr) {
        return;
    }
    
    QPdfWriter writer(fileUrl.toLocalFile());
    writer.setCreator(QStringLiteral("org.kde.skanpage"));
    writer.setTitle(title);
    QPainter painter;
    
    for (int i = 0; i < document.count(); ++i) {
        printPage(writer, painter, document.at(i), i == 0);
        m_OCREngine->OCRPage(writer, painter, document.at(i));
    }
    
    Q_EMIT showUserMessage(SkanpageUtils::InformationMessage, i18n("Document saved with OCR as PDF."));
}

void DocumentSaver::printPage(QPdfWriter &writer, QPainter &painter, const SkanpageUtils::PageProperties &page, bool firstPage)
{
    writer.setResolution(page.dpi);
    writer.setPageSize(page.pageSize);
    writer.setPageMargins(QMarginsF(0, 0, 0, 0));
    int rotationAngle = page.rotationAngle;
    if (rotationAngle == 90 || rotationAngle == 270) {
        writer.setPageOrientation(QPageLayout::Landscape);
    } else {
        writer.setPageOrientation(QPageLayout::Portrait);
    }
    writer.newPage();

    QTransform transformation;
    if (rotationAngle != 0) {
        transformation.translate(writer.width()/2, writer.height()/2);
        transformation.rotate(rotationAngle);
        transformation.translate(-writer.width()/2, -writer.height()/2);
    }
    if (rotationAngle == 90 || rotationAngle == 270) {
        //strange that this is needed and Qt does not do this automatically
        transformation.translate((writer.width()-writer.height())/2, (writer.height()-writer.width())/2);
    }

    if (firstPage) {
        painter.begin(&writer);
    }

    painter.setTransform(transformation);
    QImage pageImage(page.temporaryFile->fileName());
    painter.drawImage(QPoint(0, 0), pageImage, pageImage.rect());
}

void DocumentSaver::saveImage(const QFileInfo &fileInfo, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type)
{
    const int count = document.count();
    QImage pageImage;
    QString fileName;
    QList<QUrl> fileUrls;

    bool success = true;
    if (count == 1) {
        pageImage.load(document.at(0).temporaryFile->fileName());
        fileName = fileInfo.absoluteFilePath();
        const int rotationAngle = document.at(0).rotationAngle;
        if (rotationAngle != 0) {
            pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
        }
        success = pageImage.save(fileName, fileInfo.suffix().toLocal8Bit().constData());
        fileUrls.append(QUrl::fromLocalFile(fileName));
    } else {
        fileUrls.reserve(count);
        for (int i = 0; i < count; ++i) {
            pageImage.load(document.at(i).temporaryFile->fileName());
            const int rotationAngle = document.at(i).rotationAngle;
            if (rotationAngle != 0) {
                pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
            }
            fileName =
                QStringLiteral("%1/%2%3.%4").arg(fileInfo.absolutePath(), fileInfo.baseName(), QLocale().toString(i).rightJustified(4, QLatin1Char('0')), fileInfo.suffix());
            if(!pageImage.save(fileName, fileInfo.suffix().toLocal8Bit().constData())) {
                success = false;
            }
            fileUrls.append(QUrl::fromLocalFile(fileName));
        }
    }

    if (success) {
        if (type == SkanpageUtils::EntireDocument || type == SkanpageUtils::PageSelection) {
            Q_EMIT showUserMessage(SkanpageUtils::InformationMessage, i18n("Document saved as image."));
        }
        if (type == SkanpageUtils::EntireDocument) {
            Q_EMIT fileSaved(fileUrls, document);
        } else if (type == SkanpageUtils::SharingDocument) {
            Q_EMIT sharingFileSaved(fileUrls);
        }
    } else {
        if (type == SkanpageUtils::EntireDocument || type == SkanpageUtils::PageSelection) {
            Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Failed to save document as image."));
        }
    }
}

void DocumentSaver::saveNewPageTemporary(const int pageID, const QImage &image)
{
    const QPageSize pageSize = QPageSize(QSizeF(image.width() * 1000.0 / image.dotsPerMeterX() , image.height() * 1000.0 / image.dotsPerMeterY()), QPageSize::Millimeter);
    const int dpi = qRound(image.dotsPerMeterX() / 1000.0 * 25.4);
    QTemporaryFile *tempImageFile = new QTemporaryFile();
    tempImageFile->open();
    if (image.save(tempImageFile, "PNG")) {
        qCDebug(SKANPAGE_LOG) << "Saved new image to temporary file.";
    } else {
        qCDebug(SKANPAGE_LOG) << "Saving new image to temporary file failed!";
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Failed to save image"));
    }
    qCDebug(SKANPAGE_LOG) << image << tempImageFile << "with page size" << pageSize << "and resolution of" << dpi << "dpi";
    tempImageFile->close();
    Q_EMIT pageTemporarilySaved(pageID, {std::shared_ptr<QTemporaryFile>(tempImageFile), pageSize, dpi});
}
