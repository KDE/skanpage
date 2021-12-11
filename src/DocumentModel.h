/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DOCUMENT_MODEL_H
#define DOCUMENT_MODEL_H

#include <memory>

#include <QAbstractListModel>
#include <QString>
#include <QUrl>
#include <QSize>

#include "SkanpageUtils.h"

struct PreviewPageProperties {
    double aspectRatio;
    int previewWidth;
    int previewHeight;
    int pageID;
    bool isSaved;
};

class DocumentSaver;
class DocumentPrinter;

class DocumentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool changed READ changed NOTIFY changedChanged)
    Q_PROPERTY(int activePageIndex READ activePageIndex WRITE setActivePageIndex NOTIFY activePageChanged)
    Q_PROPERTY(QUrl activePageSource READ activePageSource NOTIFY activePageChanged)
    Q_PROPERTY(int activePageRotation READ activePageRotation NOTIFY activePageChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum DocumentModelRoles { ImageUrlRole = Qt::UserRole + 1,
        RotationAngleRole,
        AspectRatioRole,
        PreviewWidthRole,
        PreviewHeightRole,
        IsSavedRole};

    explicit DocumentModel(QObject *parent = nullptr);
    ~DocumentModel();

    const QString name() const;
    int activePageIndex() const;
    int activePageRotation() const;
    QUrl activePageSource() const;
    bool changed() const;

    void setActivePageIndex(int);

    void addImage(const QImage &image);

    Q_INVOKABLE QVariantList imageFormatNameFilter() const;

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void moveImage(int from, int to);

    Q_INVOKABLE void removeImage(int row);

    Q_INVOKABLE void rotateImage(int row, bool positiveDirection);

    Q_INVOKABLE void save(const QUrl &fileUrl, QList<int> pageNumbers = {});

    Q_INVOKABLE void print();

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void nameChanged();
    void changedChanged();
    void activePageChanged();
    void countChanged();
    void newPageAdded();
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &text);

private Q_SLOTS:
    void updateFileInformation(const QString &fileName, const SkanpageUtils::DocumentPages &document);

private:
    void updatePageInModel(const int pageID, const SkanpageUtils::PageProperties &page);

    SkanpageUtils::DocumentPages m_pages;
    QList<PreviewPageProperties> m_details;
    QString m_name;
    bool m_changed = false;
    int m_activePageIndex = -1;
    int m_idCounter = 0;
    QVariantList m_formatNameFilter;
    std::unique_ptr<DocumentSaver> m_documentSaver;
    std::unique_ptr<DocumentPrinter> m_documentPrinter;
};

#endif // DOCUMENT_MODEL_H
