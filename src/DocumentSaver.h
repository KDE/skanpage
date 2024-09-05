/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DOCUMENT_SAVER_H
#define DOCUMENT_SAVER_H

#include <memory>

#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QObject>
#include <QPainter>
#include <QPdfWriter>
#include <QUrl>

#include "SkanpageUtils.h"

class OCREngine;

class DocumentSaver : public QObject
{
    Q_OBJECT

public:
    explicit DocumentSaver(QObject *parent = nullptr);
    ~DocumentSaver() override;

    void saveDocument(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type, const QString &title);
    void saveNewPageTemporary(const int pageID, const QImage &image);
    void setOCREngine(OCREngine *engine);

Q_SIGNALS:
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &text);
    void fileSaved(const QList<QUrl> &fileUrls, const SkanpageUtils::DocumentPages &document);
    void sharingFileSaved(const QList<QUrl> &fileUrls);
    void pageTemporarilySaved(const int pageID, const SkanpageUtils::PageProperties &page);

private:
    void savePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type);
    void saveImage(const QFileInfo &fileInfo, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type);
    void saveSearchablePDF(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const QString &title);
    void printPage(QPdfWriter &writer, QPainter &painter, const SkanpageUtils::PageProperties &page, bool firstPage);
    OCREngine *m_OCREngine;
};

#endif // DOCUMENT_SAVER_H
