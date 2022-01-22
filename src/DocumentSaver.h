/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DOCUMENT_SAVER_H
#define DOCUMENT_SAVER_H

#include <memory>

#include <QObject>
#include <QFileInfo>
#include <QList>
#include <QUrl>
#include <QImage>

#include "SkanpageUtils.h"

class DocumentSaver : public QObject
{
    Q_OBJECT

public:

    explicit DocumentSaver(QObject *parent = nullptr);
    ~DocumentSaver();

    void saveDocument(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type);
    void saveNewPageTemporary(const int pageID, const QImage &image);
    
Q_SIGNALS:
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &text);
    void fileSaved(const QList<QUrl> &fileUrls, const SkanpageUtils::DocumentPages &document);
    void pageTemporarilySaved(const int pageID, const SkanpageUtils::PageProperties &page);

private:
    void savePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type);
    void saveImage(const QFileInfo &fileInfo, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type);
};

#endif // DOCUMENT_SAVER_H
