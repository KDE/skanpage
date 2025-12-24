/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DocumentSaver.h"

#include <QTransform>

#include <KLocalizedString>

#include "OCREngine.h"
#include "skanpage_debug.h"

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

    const QFileInfo &fileInfo = QFileInfo(fileUrl.toString(QUrl::PreferLocalFile));
    const QString &fileSuffix = fileInfo.suffix();

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Selected file suffix is") << fileSuffix;

    if (type == SkanpageUtils::OCRDocument) {
        m_OCREngine->InitForOCR();
        saveSearchablePDF(fileUrl, document, title);
    } else if (fileSuffix == QLatin1String("pdf") || fileSuffix.isEmpty()) {
        savePDF(fileUrl, document, title, type);
    } else {
        saveImage(fileUrl, fileInfo, document, type);
    }
}

QString DocumentSaver::getLocalNameForFile(const QUrl &fileUrl)
{
    QString localName;
    if (!fileUrl.isLocalFile()) {
        QTemporaryFile tmp;
        bool ok = tmp.open();
        if (!ok) {
            qCDebug(SKANPAGE_LOG) << QStringLiteral("Could not open file to write image data to temporary file.");
            return QString();
        }
        localName = tmp.fileName();
        tmp.close(); // we just want the filename
    }
    else {
        localName = fileUrl.toLocalFile();
    }

    return localName;
}

void DocumentSaver::savePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const QString &title, const SkanpageUtils::FileType type)
{
    const QString localName = getLocalNameForFile(fileUrl);
    QFile file(localName);
    bool ok = file.open(QIODevice::WriteOnly);
    if (!ok) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18nc("%1 is the error message", "An error ocurred while saving: %1.", file.errorString()));
        return;
    }
    QPdfWriter writer(&file);
    writer.setCreator(QStringLiteral("org.kde.skanpage"));
    writer.setTitle(title);
    QPainter painter;

    for (int i = 0; i < document.count(); ++i) {
        Q_EMIT showUserMessage(SkanpageUtils::InformationMessage, i18nc("indicate status update during saving", "Processing page %1 to save the document.", i));
        printPage(writer, painter, document.at(i), i == 0);
    }

    if (type == SkanpageUtils::EntireDocument || type == SkanpageUtils::PageSelection) {
        Q_EMIT showUserMessage(SkanpageUtils::InformationMessage,
                               i18nc("@info: user message to confirm the save process and the filename", "Document saved as '%1'.", fileUrl.fileName()));
    }

    if (type == SkanpageUtils::EntireDocument) {
        Q_EMIT fileSaved({fileUrl}, {localName}, document);
    } else if (type == SkanpageUtils::SharingDocument) {
        Q_EMIT sharingFileSaved({fileUrl});
    }
}

void DocumentSaver::saveSearchablePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const QString &title)
{
    if (m_OCREngine == nullptr) {
        return;
    }

    const QString localName = getLocalNameForFile(fileUrl);
    QFile file(localName);
    bool ok = file.open(QIODevice::ReadWrite);
    if (!ok) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18nc("%1 is the error message", "An error ocurred while saving: %1.", file.errorString()));
        return;
    }
    QPdfWriter writer(&file);
    writer.setCreator(QStringLiteral("org.kde.skanpage"));
    writer.setTitle(title);
    QPainter painter;

    for (int i = 0; i < document.count(); ++i) {
        Q_EMIT showUserMessage(SkanpageUtils::InformationMessage, i18nc("indicate status update during saving", "Processing page %1 to save the document.", i));
        printPage(writer, painter, document.at(i), i == 0);
        m_OCREngine->OCRPage(writer, painter, document.at(i));
    }

    Q_EMIT fileSaved({fileUrl}, {localName}, document);
    Q_EMIT showUserMessage(
        SkanpageUtils::InformationMessage,
        i18nc("@info: user message to confirm the OCR and save process and the filename", "Document saved with OCR as '%1'.", fileUrl.fileName()));
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
        transformation.translate(writer.width() / 2, writer.height() / 2);
        transformation.rotate(rotationAngle);
        transformation.translate(-writer.width() / 2, -writer.height() / 2);
    }
    if (rotationAngle == 90 || rotationAngle == 270) {
        // strange that this is needed and Qt does not do this automatically
        transformation.translate((writer.width() - writer.height()) / 2, (writer.height() - writer.width()) / 2);
    }

    if (firstPage) {
        painter.begin(&writer);
    }

    painter.setTransform(transformation);
    QImage pageImage;
    if (page.temporaryFile != nullptr) {
        pageImage.load(page.temporaryFile->fileName());
    }
    painter.drawImage(QPoint(0, 0), pageImage, pageImage.rect());
}

void DocumentSaver::saveImage(const QUrl &fileUrl, const QFileInfo &destFileInfo, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type)
{
    const int count = document.count();
    QImage pageImage;
    QString localFileName;
    QString destFileName;
    QList<QUrl> fileUrls;
    QList<QString> localNames;

    bool success = true;
    if (count == 1) {
        if (document.at(0).temporaryFile != nullptr) {
            QFileInfo localFileInfo(getLocalNameForFile(fileUrl));
            pageImage.load(document.at(0).temporaryFile->fileName());
            localFileName = localFileInfo.absoluteFilePath();
            const int rotationAngle = document.at(0).rotationAngle;
            if (rotationAngle != 0) {
                pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
            }
            success = pageImage.save(localFileName, destFileInfo.suffix().toLocal8Bit().constData());
            fileUrls.append(fileUrl);
            localNames.append((localFileName));
        }
    } else {
        fileUrls.reserve(count);
        localNames.reserve(count);

        for (int i = 0; i < count; ++i) {
            Q_EMIT showUserMessage(SkanpageUtils::InformationMessage,
                                   i18nc("indicate status update during saving", "Processing page %1 to save the document.", i));
            if (document.at(i).temporaryFile == nullptr) {
                continue;
            }
            QFileInfo localFileInfo(getLocalNameForFile(fileUrl));
            pageImage.load(document.at(i).temporaryFile->fileName());
            const int rotationAngle = document.at(i).rotationAngle;
            if (rotationAngle != 0) {
                pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
            }
            destFileName =
                QStringLiteral("%1/%2-%3.%4").arg(destFileInfo.path(), destFileInfo.baseName(), QLocale().toString(i).rightJustified(4, QLatin1Char('0')), destFileInfo.suffix());

            if (fileUrl.isLocalFile()) {
                localFileName = destFileName;
            } else {
                localFileName = localFileInfo.absoluteFilePath();
            }

            if (!pageImage.save(localFileName, destFileInfo.suffix().toLocal8Bit().constData())) {
                success = false;
            }
            fileUrls.append(QUrl::fromUserInput(destFileName));
            localNames.append(localFileName);
        }
    }

    if (success) {
        if (type == SkanpageUtils::EntireDocument || type == SkanpageUtils::PageSelection) {
            Q_EMIT showUserMessage(SkanpageUtils::InformationMessage,
                                   i18nc("@info: user message to confirm the save process and the filename", "Document saved as '%1'.", fileUrl.fileName()));
        }
        if (type == SkanpageUtils::EntireDocument) {
            Q_EMIT fileSaved(fileUrls, localNames, document);
        } else if (type == SkanpageUtils::SharingDocument) {
            Q_EMIT sharingFileSaved(fileUrls);
        }
    } else {
        if (type == SkanpageUtils::EntireDocument || type == SkanpageUtils::PageSelection) {
            Q_EMIT showUserMessage(
                SkanpageUtils::ErrorMessage,
                i18nc("@info: user message to inform that the save process has failed", "Failed to save document as '%1'.", fileUrl.fileName()));
        }
    }
}

void DocumentSaver::saveNewPageTemporary(const int pageID, const QImage &image)
{
    const QPageSize pageSize =
        QPageSize(QSizeF(image.width() * 1000.0 / image.dotsPerMeterX(), image.height() * 1000.0 / image.dotsPerMeterY()), QPageSize::Millimeter);
    const int dpi = qRound(image.dotsPerMeterX() / 1000.0 * 25.4);
    QTemporaryFile *tempImageFile = new QTemporaryFile();
    bool ok = tempImageFile->open();
    if (!ok) {
        qCDebug(SKANPAGE_LOG) << "Failed to open temporary file to save a new image";
        return;
    }
    if (image.save(tempImageFile, "PNG")) {
        qCDebug(SKANPAGE_LOG) << "Saved new image to temporary file.";
    } else {
        qCDebug(SKANPAGE_LOG) << "Saving new image to temporary file failed!";
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Failed to save image."));
    }
    qCDebug(SKANPAGE_LOG) << image << tempImageFile << "with page size" << pageSize << "and resolution of" << dpi << "dpi";
    tempImageFile->close();
    Q_EMIT pageTemporarilySaved(pageID, {std::shared_ptr<QTemporaryFile>(tempImageFile), pageSize, dpi});
}

#include "moc_DocumentSaver.cpp"
