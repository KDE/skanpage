/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DocumentSaver_H
#define DocumentSaver_H

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

    explicit DocumentSaver(QObject *parent = nullptr);
    ~DocumentSaver();

    void saveDocument(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document);
    void saveNewPageTemporary(const int pageID, const QImage &image);
    
Q_SIGNALS:
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &text);
    void fileSaved(const QString &fileName, const SkanpageUtils::DocumentPages &document);
    void pageTemporarilySaved(const int pageID, const SkanpageUtils::PageProperties &page);

private:
    void save(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document);
    void savePDF(const QString &filePath, const SkanpageUtils::DocumentPages &document);
    bool saveImage(const QFileInfo &fileInfo, const SkanpageUtils::DocumentPages &document);
    void saveNewPage(const int pageID, const QImage &image);
    
    QFuture<void> m_future;
};

#endif
