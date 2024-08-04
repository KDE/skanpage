/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef IMAGE_IMPORT_H
#define IMAGE_IMPORT_H

#include <QObject>
#include <QList>
#include <QFileInfo>
#include <QUrl>

#include "SkanpageUtils.h"


class ImageImport : public QObject
{
    Q_OBJECT

public:

    explicit ImageImport(QObject *parent = nullptr);
    ~ImageImport() override;
    void importImageFile(const QUrl &fileUrl, const int currentDPI);
Q_SIGNALS:
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &text);
    void imageImported(const QImage &image);
private:
    void importPDF(const QUrl &fileUrl, const int dotsPerMeter);
};

#endif // IMAGE_IMPORT_H
