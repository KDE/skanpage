/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DocumentModel.h"

#include <QImage>
#include <QSet>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>

#include <QApplication>

#include <KLocalizedString>
#include <KIO/FileCopyJob>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>

#include "skanpage_debug.h"

struct PreviewPageProperties {
    double aspectRatio;
    int previewWidth;
    int previewHeight;
    int pageID;
    bool isSaved;
};

QDebug operator<<(QDebug d, const PreviewPageProperties &pageProperties)
{
    d << "ID: " << pageProperties.pageID << "\n";
    d << "Aspect ratio: " << pageProperties.aspectRatio << "\n";
    d << "Preview width: " << pageProperties.previewWidth << "\n";
    d << "Preview height: " << pageProperties.previewHeight << "\n";
    d << "Is saved: " << pageProperties.isSaved << "\n";
    return d;
}

class DocumentModelPrivate
{
public:
    explicit DocumentModelPrivate();

    SkanpageUtils::DocumentPages m_pages;
    QList<PreviewPageProperties> m_details;
    QList<QUrl> m_fileUrls;
    bool m_changed = false;
    int m_activePageIndex = -1;
    int m_idCounter = 0;
    const QString m_defaultFileName;
    QSet<int> m_nonSavedPages;
};

DocumentModelPrivate::DocumentModelPrivate()
    : m_defaultFileName(i18n("New document"))
{
}

DocumentModel::DocumentModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::make_unique<DocumentModelPrivate>())
{
}

DocumentModel::~DocumentModel()
{
}

QString DocumentModel::name() const
{
    if (d->m_fileUrls.isEmpty()) {
        return i18n("New document");
    }
    if (d->m_fileUrls.count() > 1) {
        return i18nc("for file names, indicates a range: from file0000.png to file0014.png",
                     "%1 ... %2",
                     d->m_fileUrls.first().fileName(),
                     d->m_fileUrls.last().fileName());
    }
    return d->m_fileUrls.first().fileName();
}

QString DocumentModel::fileName() const
{
    if (d->m_fileUrls.isEmpty() || d->m_fileUrls.count() > 1) {
        return QString(i18n("New document") + QStringLiteral(".pdf"));
    }
    return d->m_fileUrls.first().fileName();
}

bool DocumentModel::changed() const
{
    return d->m_changed;
}

bool DocumentModel::isReady() const
{
    return d->m_nonSavedPages.isEmpty();
}

int DocumentModel::activePageIndex() const
{
    return d->m_activePageIndex;
}

int DocumentModel::activePageRotation() const
{
    if (d->m_activePageIndex >= 0 && d->m_activePageIndex < rowCount()) {
        return d->m_pages.at(d->m_activePageIndex).rotationAngle;
    }
    return 0;
}

QUrl DocumentModel::activePageSource() const
{
    return data(index(d->m_activePageIndex, 0), ImageUrlRole).toUrl();
}

void DocumentModel::setActivePageIndex(int newIndex)
{
    if (newIndex != d->m_activePageIndex && newIndex >= 0 && newIndex < rowCount()) {
        d->m_activePageIndex = newIndex;
        Q_EMIT activePageChanged();
    }
}

void DocumentModel::save(const QUrl &fileUrl, const QList<int> &pageNumbers)
{
    if (pageNumbers.isEmpty()) {
        Q_EMIT saveDocument(fileUrl, d->m_pages);
    } else {
        Q_EMIT saveDocument(fileUrl, selectPages(pageNumbers), SkanpageUtils::PageSelection);
    }
}

void DocumentModel::createSharingFile(const QString &suffix, const QList<int> &pageNumbers)
{
    if (d->m_pages.isEmpty()) {
        return;
    }

    const QUrl temporaryLocation = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QStringLiteral("/document.") + suffix);
    Q_EMIT saveDocument(temporaryLocation, selectPages(pageNumbers), SkanpageUtils::SharingDocument);
}

void DocumentModel::exportPDF(const QUrl &fileUrl, const QString &title, const bool useOCR)
{
    if (useOCR) {
        Q_EMIT saveDocument(fileUrl, d->m_pages, SkanpageUtils::OCRDocument, title);
    } else {
        Q_EMIT saveDocument(fileUrl, d->m_pages, SkanpageUtils::EntireDocument, title);
    }
}

void DocumentModel::addImage(const QImage &image)
{
    const int oldSize = d->m_nonSavedPages.size();
    d->m_nonSavedPages.insert(d->m_idCounter);
    if (oldSize == 0 && d->m_nonSavedPages.size() > 0) {
        Q_EMIT isReadyChanged(false);
    }

    const double aspectRatio = static_cast<double>(image.height()) / image.width();
    beginInsertRows(QModelIndex(), d->m_pages.count(), d->m_pages.count());
    const PreviewPageProperties newPage = {aspectRatio, 500, static_cast<int>(500 * aspectRatio), d->m_idCounter, false};

    d->m_idCounter++;
    qCDebug(SKANPAGE_LOG) << "Inserting new page into model:" << newPage;
    d->m_details.append(newPage);
    d->m_pages.append({nullptr, QPageSize(), 0});
    endInsertRows();
    Q_EMIT countChanged();
    Q_EMIT saveNewPageTemporary(newPage.pageID, image);
}

void DocumentModel::updatePageInModel(const int pageID, const SkanpageUtils::PageProperties &page)
{
    if (d->m_details.count() <= 0) {
        return;
    }
    /* Most likely, the updated page is the last one in the model
     * unless the user has deleted a page between the finished scanning and the
     * processing. Thus try this first and look for the page ID if this is not the case. */
    int pageIndex = d->m_details.count() - 1;
    if (d->m_details.at(pageIndex).pageID != pageID) {
        for (int i = d->m_details.count() - 1; i >= 0; i--) {
            if (d->m_details.at(i).pageID == pageID) {
                pageIndex = i;
                break;
            }
        }
    }
    d->m_pages[pageIndex].dpi = page.dpi;
    d->m_pages[pageIndex].temporaryFile = page.temporaryFile;
    d->m_pages[pageIndex].pageSize = page.pageSize;
    d->m_details[pageIndex].isSaved = true;
    d->m_nonSavedPages.remove(pageID);
    if (d->m_nonSavedPages.isEmpty()) {
        Q_EMIT isReadyChanged(true);
    }
    Q_EMIT dataChanged(index(pageIndex, 0), index(pageIndex, 0), {ImageUrlRole, IsSavedRole});

    if (!d->m_changed) {
        d->m_changed = true;
        Q_EMIT changedChanged();
    }

    d->m_activePageIndex = pageIndex;
    Q_EMIT activePageChanged();
    Q_EMIT newPageAdded();
}

void DocumentModel::moveImage(int from, int to)
{
    int add = 0;
    if (from == to) {
        return;
    }
    if (to > from) {
        add = 1;
    }
    if (from < 0 || from >= d->m_pages.count()) {
        return;
    }
    if (to < 0 || to >= d->m_pages.count()) {
        return;
    }

    bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), to + add);
    if (!ok) {
        qCDebug(SKANPAGE_LOG) << "Failed to move" << from << to << add << d->m_pages.count();
        return;
    }
    d->m_pages.move(from, to);
    d->m_details.move(from, to);
    endMoveRows();

    if (d->m_activePageIndex == from) {
        d->m_activePageIndex = to;
    } else if (d->m_activePageIndex == to) {
        d->m_activePageIndex = from;
    }
    Q_EMIT activePageChanged();

    if (!d->m_changed) {
        d->m_changed = true;
        Q_EMIT changedChanged();
    }
}

void DocumentModel::rotateImage(int row, RotateOption rotate)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }
    int rotationAngle = d->m_pages.at(row).rotationAngle;
    if (rotate == RotateOption::Rotate90positive) {
        rotationAngle += 90;
    } else if (rotate == RotateOption::Flip180) {
        rotationAngle += 180;
    } else {
        rotationAngle -= 90;
    }
    if (rotationAngle < 0) {
        rotationAngle = rotationAngle + 360;
    } else if (rotationAngle >= 360) {
        rotationAngle = rotationAngle - 360;
    }
    d->m_pages[row].rotationAngle = rotationAngle;
    if (row == d->m_activePageIndex) {
        Q_EMIT activePageChanged();
    }
    Q_EMIT dataChanged(index(row, 0), index(row, 0), {RotationAngleRole});
}

void DocumentModel::flipPages(FlipPagesOption flip)
{
    int start = 0;
    int increment = 1;
    if (flip == FlipPagesOption::FlipEven) {
        start = 1;
    }
    if (flip == FlipPagesOption::FlipEven || flip == FlipPagesOption::FlipOdd) {
        increment = 2;
    }
    for (int i = start; i < d->m_pages.count(); i += increment) {
        int rotationAngle = d->m_pages.at(i).rotationAngle;
        rotationAngle += 180;
        if (rotationAngle < 0) {
            rotationAngle = rotationAngle + 360;
        } else if (rotationAngle >= 360) {
            rotationAngle = rotationAngle - 360;
        }
        d->m_pages[i].rotationAngle = rotationAngle;
    }
    Q_EMIT activePageChanged();
    Q_EMIT dataChanged(index(0, 0), index(d->m_pages.count() - 1, 0), {RotationAngleRole});
}

void DocumentModel::reorderPages(ReorderOption reorder)
{
    Q_EMIT layoutAboutToBeChanged();
    if (reorder == Reverse) {
        std::reverse(d->m_pages.begin(), d->m_pages.end());
        std::reverse(d->m_details.begin(), d->m_details.end());
    } else {
        SkanpageUtils::DocumentPages newPages;
        QList<PreviewPageProperties> newDetails;
        newPages.reserve(d->m_pages.count());
        newDetails.reserve(d->m_pages.count());
        const int halfStart = d->m_pages.count() / 2;
        const int oddOffset = d->m_pages.count() % 2;
        if (reorder == ReorderDuplex) {
            for (int i = 0; i < halfStart; i++) {
                newPages.append(d->m_pages.at(i));
                newPages.append(d->m_pages.at(i + halfStart + oddOffset));
                newDetails.append(d->m_details.at(i));
                newDetails.append(d->m_details.at(i + halfStart + oddOffset));
            }
        } else {
            for (int i = 0; i < halfStart; i++) {
                newPages.append(d->m_pages.at(i));
                newPages.append(d->m_pages.at(d->m_pages.count() - 1 - i));
                newDetails.append(d->m_details.at(i));
                newDetails.append(d->m_details.at(d->m_details.count() - 1 - i));
            }
        }
        if (oddOffset == 1) {
            newPages.append(d->m_pages.at(halfStart));
            newDetails.append(d->m_details.at(halfStart));
        };
        d->m_pages = newPages;
        d->m_details = newDetails;
    }
    Q_EMIT layoutChanged();
}

void DocumentModel::removeImage(int row)
{
    if (row < 0 || row >= d->m_pages.count()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    d->m_pages.removeAt(row);
    d->m_details.removeAt(row);
    endRemoveRows();

    if (row < d->m_activePageIndex) {
        d->m_activePageIndex -= 1;
    } else if (d->m_activePageIndex >= d->m_pages.count()) {
        d->m_activePageIndex = d->m_pages.count() - 1;
    }
    Q_EMIT activePageChanged();
    Q_EMIT countChanged();

    if (!d->m_changed) {
        d->m_changed = true;
        Q_EMIT changedChanged();
    }
}

QHash<int, QByteArray> DocumentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageUrlRole] = "imageUrl";
    roles[RotationAngleRole] = "rotationAngle";
    roles[IsSavedRole] = "isSaved";
    roles[PreviewWidthRole] = "previewWidth";
    roles[PreviewHeightRole] = "previewHeight";
    roles[AspectRatioRole] = "aspectRatio";
    return roles;
}

int DocumentModel::rowCount(const QModelIndex &) const
{
    return d->m_pages.count();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= d->m_pages.size() || index.row() < 0) {
        return QVariant();
    }

    switch (role) {
    case ImageUrlRole:
        if (d->m_details.at(index.row()).isSaved || d->m_pages.at(index.row()).temporaryFile.get() != nullptr) {
            return QUrl::fromLocalFile(d->m_pages.at(index.row()).temporaryFile->fileName());
        } else {
            return QUrl();
        }
    case RotationAngleRole:
        return d->m_pages.at(index.row()).rotationAngle;
    case IsSavedRole:
        return d->m_details.at(index.row()).isSaved;
    case AspectRatioRole:
        return d->m_details.at(index.row()).aspectRatio;
    case PreviewWidthRole:
        return d->m_details.at(index.row()).previewWidth;
    case PreviewHeightRole:
        return d->m_details.at(index.row()).previewHeight;
    }
    return QVariant();
}

void DocumentModel::clearData()
{
    beginResetModel();
    d->m_pages.clear();
    d->m_details.clear();
    d->m_activePageIndex = -1;
    endResetModel();
    Q_EMIT countChanged();

    if (!d->m_fileUrls.isEmpty() && d->m_defaultFileName != d->m_fileUrls.first().fileName()) {
        d->m_fileUrls.first() = QUrl::fromLocalFile(d->m_defaultFileName);
        Q_EMIT nameChanged();
    }
    if (d->m_changed) {
        d->m_changed = false;
        Q_EMIT changedChanged();
    }
}

void DocumentModel::updateFileInformation(const QList<QUrl> &fileUrls, const QList<QString> &localNames, const SkanpageUtils::DocumentPages &document)
{
    for (int i = 0; i < fileUrls.count(); i++) {
        if (!fileUrls.at(i).isLocalFile()) {
            QFile file(localNames.at(i));
            bool okOpen = file.open(QIODevice::ReadOnly);
            if (!okOpen) {
                qCDebug(SKANPAGE_LOG) << "Failed to open local file during copying to remote location.";
            }
            const auto copyJob = KIO::storedPut(&file, fileUrls.at(i), -1, KIO::HideProgressInfo | KIO::Overwrite);
            KJobWidgets::setWindow(copyJob, QApplication::activeWindow());
            bool okCopy = copyJob->exec();

            if (!okCopy) {
                file.close();
                file.remove();
                qCDebug(SKANPAGE_LOG) << "Failed to copy file to remote location.";
                return;
            }
        }
    }
    if (document == d->m_pages && d->m_changed) {
        d->m_changed = false;
        Q_EMIT changedChanged();
    }

    if (d->m_fileUrls != fileUrls) {
        d->m_fileUrls = fileUrls;
        Q_EMIT nameChanged();
    }
}

void DocumentModel::updateSharingFileInformation(const QList<QUrl> &fileUrls)
{
    QVariantList temp;
    for (const auto &url : fileUrls) {
        temp << url.toString();
    }
    Q_EMIT sharingDocumentsCreated(temp);
}

SkanpageUtils::DocumentPages DocumentModel::selectPages(QList<int> pageNumbers) const
{
    if (pageNumbers.isEmpty()) {
        return d->m_pages;
    }

    SkanpageUtils::DocumentPages document;
    std::sort(pageNumbers.begin(), pageNumbers.end());
    for (int i = 0; i < pageNumbers.count(); i++) {
        const int page = pageNumbers.at(i);
        if (page >= 0 && page < d->m_pages.count()) {
            document.append(d->m_pages.at(pageNumbers.at(i)));
        }
    }
    return document;
}

#include "moc_DocumentModel.cpp"
