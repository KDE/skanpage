/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DOCUMENT_SAVER_H
#define DOCUMENT_SAVER_H

#include <memory>

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QFuture>
#include <QImage>

#include "SkanpageUtils.h"

class DocumentSaver : public QObject
{
    Q_OBJECT

public:

    enum FileType {
        EntireDocument,
        PageSelection};

    explicit DocumentSaver(QObject *parent = nullptr);
    ~DocumentSaver();

    void saveDocument(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const FileType type = EntireDocument);
    void saveNewPageTemporary(const int pageID, const QImage &image);
    
Q_SIGNALS:
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &text);
    void fileSaved(const QList<QUrl> &fileUrls, const SkanpageUtils::DocumentPages &document);
    void pageTemporarilySaved(const int pageID, const SkanpageUtils::PageProperties &page);

private:
    void save(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const FileType type);
    void savePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const FileType type);
    void saveImage(const QFileInfo &fileInfo, const SkanpageUtils::DocumentPages &document, const FileType type);
    void saveNewPage(const int pageID, const QImage &image);
    
    QFuture<void> m_future;
};

#endif // DOCUMENT_SAVER_H
