/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "ImageImport.h"

#include <QPainter>
#include <QPdfDocument>

#include <KLocalizedString>

#include "skanpage_debug.h"

ImageImport::ImageImport(QObject *parent)
    : QObject(parent)
{
}

ImageImport::~ImageImport()
{
}

void ImageImport::importImageFile(const QUrl &fileUrl, const int currentDPI)
{
    const QFileInfo &fileInfo = QFileInfo(fileUrl.toLocalFile());
    const QString &fileSuffix = fileInfo.suffix();

    qCDebug(SKANPAGE_LOG) << QStringLiteral("File suffix is") << fileSuffix;

    if (fileSuffix == QLatin1String("pdf") || fileSuffix.isEmpty()) {
        importPDF(fileUrl, currentDPI);
    } else {
        QImage image;
        if (image.load(fileInfo.filePath())) {
            const int dotsPerMeter = qRound(currentDPI * 1000.0 / 25.4);
            image.setDotsPerMeterX(dotsPerMeter);
            image.setDotsPerMeterY(dotsPerMeter);
            Q_EMIT imageImported(image);
        } else {
            Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Failed to load image!"));
        }
    }
}

void ImageImport::importPDF(const QUrl &fileUrl, const int currentDPI)
{
    QPainter painter;
    QPdfDocument document;
    QPdfDocument::Error error = document.load(fileUrl.toLocalFile());
    if (error != QPdfDocument::Error::None) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Failed to load PDF file!"));
        return;
    }

    for (int i = 0; i < document.pageCount(); ++i) {
        // pageSize is given in points = 1 / 72 inch = 0.3527777778 mm
        QSizeF sizeF = document.pagePointSize(i) * 0.3527777778f * currentDPI / 25.4f;
        QSize size = sizeF.toSize();
        QImage page(size, QImage::Format_RGB32);
        const int dotsPerMeter = qRound(currentDPI * 1000.0 / 25.4);
        page.setDotsPerMeterX(dotsPerMeter);
        page.setDotsPerMeterY(dotsPerMeter);
        page.fill(QColorConstants::White);
        painter.begin(&page);
        painter.drawImage(0, 0, document.render(i, size));
        painter.end();
        Q_EMIT imageImported(page);
    }
}

#include "moc_ImageImport.cpp"
