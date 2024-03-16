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

#include "SkanpageUtils.h"

class DocumentModelPrivate;

class DocumentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY nameChanged)
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

    enum RotateOption { Rotate90positive,
        Rotate90negative,
        Flip180};

    Q_ENUM(RotateOption)

    enum FlipPagesOption { FlipEven,
        FlipOdd,
        FlipAll};

    Q_ENUM(FlipPagesOption)

    enum ReorderOption { ReorderDuplex,
        ReorderDuplexReversed,
        Reverse};

    Q_ENUM(ReorderOption)

    explicit DocumentModel(QObject *parent = nullptr);
    ~DocumentModel();

    QString name() const;
    QString fileName() const;
    int activePageIndex() const;
    int activePageRotation() const;
    QUrl activePageSource() const;
    bool changed() const;

    void setActivePageIndex(int);

    void addImage(const QImage &image);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void moveImage(int from, int to);

    Q_INVOKABLE void removeImage(int row);

    Q_INVOKABLE void rotateImage(int row, RotateOption rotate = RotateOption::Rotate90positive);

    Q_INVOKABLE void flipPages(FlipPagesOption flip);

    Q_INVOKABLE void reorderPages(ReorderOption reorder);

    Q_INVOKABLE void save(const QUrl &fileUrl, const QList<int> &pageNumbers = {});

    Q_INVOKABLE void createSharingFile(const QString &suffix, const QList<int> &pageNumbers = {});

    Q_INVOKABLE void exportPDF(const QUrl &fileUrl, const QString &title, const bool useOCR);

    SkanpageUtils::DocumentPages selectPages(QList<int> pageNumbers) const;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void nameChanged();
    void changedChanged();
    void activePageChanged();
    void countChanged();
    void newPageAdded();
    void saveDocument(const QUrl &fileUrl, const SkanpageUtils::DocumentPages &document, const SkanpageUtils::FileType type = SkanpageUtils::EntireDocument, const QString &title = QString());
    void saveNewPageTemporary(const int pageID, const QImage &image);
    void sharingDocumentsCreated(const QVariantList &fileUrls);

public Q_SLOTS:
    void updateFileInformation(const QList<QUrl> &fileUrls, const SkanpageUtils::DocumentPages &document);
    void updateSharingFileInformation(const QList<QUrl> &fileUrls);
    void updatePageInModel(const int pageID, const SkanpageUtils::PageProperties &page);

private:
    std::unique_ptr<DocumentModelPrivate> d;
};

#endif // DOCUMENT_MODEL_H
