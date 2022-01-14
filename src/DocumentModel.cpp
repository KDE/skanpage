/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DocumentModel.h"

#include <QUrl>
#include <QTemporaryFile>
#include <QImage>
#include <QImageWriter>
#include <QMimeType>
#include <QMimeDatabase>

#include <KLocalizedString>

#include "skanpage_debug.h"
#include "DocumentSaver.h"
#include "DocumentPrinter.h"

QDebug operator<<(QDebug d, const PreviewPageProperties& pageProperties)
{
    d << "ID: " << pageProperties.pageID << "\n";
    d << "Aspect ratio: " << pageProperties.aspectRatio << "\n";
    d << "Preview width: " << pageProperties.previewWidth << "\n";
    d << "Preview height: " << pageProperties.previewHeight << "\n";
    d << "Is saved: " << pageProperties.isSaved << "\n";
    return d;
}

const static QString defaultFileName = i18n("New document");

DocumentModel::DocumentModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_name(i18n("New document"))
    , m_documentSaver(std::make_unique<DocumentSaver>())
    , m_documentPrinter(std::make_unique<DocumentPrinter>())
{
    QList<QByteArray> tempList = QImageWriter::supportedMimeTypes();
    const QMimeDatabase mimeDB;
    // Put first class citizens at first place
    tempList.removeAll(QByteArray("image/jpeg"));
    tempList.removeAll(QByteArray("image/tiff"));
    tempList.removeAll(QByteArray("image/png"));
    tempList.insert(0, QByteArray("application/pdf"));
    tempList.insert(1, QByteArray("image/png"));
    tempList.insert(2, QByteArray("image/jpeg"));
    tempList.insert(3, QByteArray("image/tiff"));

    m_formatNameFilter.append({i18nc("Description for the name filter of a QML file dialog", "All files") + QStringLiteral("(*)")});

    for (const auto &mimeString : tempList) {
        const QMimeType mimeType = mimeDB.mimeTypeForName(QString::fromLatin1(mimeString));
        // craft a string that QML's FileDialog understands
        m_formatNameFilter.append({mimeType.comment() + QStringLiteral("(*.") + mimeType.preferredSuffix() + QStringLiteral(")")});
    }

    connect(m_documentSaver.get(), &DocumentSaver::pageTemporarilySaved, this, &DocumentModel::updatePageInModel);
    connect(m_documentSaver.get(), &DocumentSaver::showUserMessage, this, &DocumentModel::showUserMessage);
    connect(m_documentSaver.get(), &DocumentSaver::fileSaved, this, &DocumentModel::updateFileInformation);
    connect(m_documentPrinter.get(), &DocumentPrinter::showUserMessage, this, &DocumentModel::showUserMessage);
}

DocumentModel::~DocumentModel()
{
}

QVariantList DocumentModel::imageFormatNameFilter() const
{
    return m_formatNameFilter;
}

const QString DocumentModel::name() const
{
    return m_name;
}

bool DocumentModel::changed() const
{
    return m_changed;
}

int DocumentModel::activePageIndex() const
{
    return m_activePageIndex;
}

int DocumentModel::activePageRotation() const
{
    if (m_activePageIndex >= 0 && m_activePageIndex < rowCount()) {
        return m_pages.at(m_activePageIndex).rotationAngle;
    }
    return 0;
}

QUrl DocumentModel::activePageSource() const
{
    return data(index(m_activePageIndex, 0), ImageUrlRole).toUrl();
}

void DocumentModel::setActivePageIndex(int newIndex)
{
    if (newIndex != m_activePageIndex) {
        m_activePageIndex = newIndex;
        Q_EMIT activePageChanged();
    }
}

void DocumentModel::save(const QUrl &fileUrl, QList<int> pageNumbers)
{
    if (pageNumbers.isEmpty()) {
        m_documentSaver->saveDocument(fileUrl, m_pages);
    } else {
        SkanpageUtils::DocumentPages doc;
        std::sort(pageNumbers.begin(), pageNumbers.end());
        for (int i = 0; i < pageNumbers.count(); i++) {
            const int page = pageNumbers.at(i);
            if (page >= 0 && page <  m_pages.count()) {
                doc.append(m_pages.at(pageNumbers.at(i)));
            }
        }
        m_documentSaver->saveDocument(fileUrl, doc, DocumentSaver::PageSelection);
    }
}

void DocumentModel::print()
{
    m_documentPrinter->printDocument(m_pages);
}

void DocumentModel::addImage(const QImage &image)
{
    const double aspectRatio = static_cast<double>(image.height())/image.width();
    beginInsertRows(QModelIndex(), m_pages.count(), m_pages.count());
    const PreviewPageProperties newPage = {aspectRatio, 500, static_cast<int>(500 * aspectRatio), m_idCounter++, false};
    qCDebug(SKANPAGE_LOG) << "Inserting new page into model:" << newPage;
    m_details.append(newPage);
    m_pages.append({nullptr, QPageSize(), 0});
    endInsertRows();
    Q_EMIT countChanged();
    m_documentSaver->saveNewPageTemporary(newPage.pageID, image);
}

void DocumentModel::updatePageInModel(const int pageID, const SkanpageUtils::PageProperties &page)
{
    if (m_details.count() <= 0) {
        return;
    }
    /* Most likely, the updated page is the last one in the model
     * unless the user has deleted a page between the finished scanning and the
     * processing. Thus try this first and look for the page ID if this is not the case. */
    int pageIndex = m_details.count() - 1;
    if (m_details.at(pageIndex).pageID != pageID) {
        for (int i = m_details.count() - 1; i >= 0; i--) {
            if (m_details.at(i).pageID == pageID) {
                pageIndex = i;
                break;
            }
        }
    }
    m_pages[pageIndex].dpi = page.dpi;
    m_pages[pageIndex].temporaryFile = page.temporaryFile;
    m_pages[pageIndex].pageSize = page.pageSize;
    m_details[pageIndex].isSaved = true;
    Q_EMIT dataChanged(index(pageIndex, 0), index(pageIndex, 0), {ImageUrlRole, IsSavedRole});

    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }

    m_activePageIndex = pageIndex;
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
    if (from < 0 || from >= m_pages.count()) {
        return;
    }
    if (to < 0 || to >= m_pages.count()) {
        return;
    }

    bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), to + add);
    if (!ok) {
        qCDebug(SKANPAGE_LOG) << "Failed to move" << from << to << add << m_pages.count();
        return;
    }
    m_pages.move(from, to);
    m_details.move(from, to);
    endMoveRows();

    if (m_activePageIndex == from) {
        m_activePageIndex = to;
    } else if (m_activePageIndex == to) {
        m_activePageIndex = from;
    }
    Q_EMIT activePageChanged();

    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }
}

void DocumentModel::rotateImage(int row, RotateOption rotate)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }
    int rotationAngle = m_pages.at(row).rotationAngle;
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
    m_pages[row].rotationAngle = rotationAngle;
    if (row == m_activePageIndex) {
        Q_EMIT activePageChanged();
    }
    Q_EMIT dataChanged(index(row, 0), index(row, 0), {RotationAngleRole});
}

void DocumentModel::removeImage(int row)
{
    if (row < 0 || row >= m_pages.count()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_pages.removeAt(row);
    m_details.removeAt(row);
    endRemoveRows();

    if (row < m_activePageIndex) {
        m_activePageIndex -= 1;
    } else if (m_activePageIndex >= m_pages.count()) {
        m_activePageIndex = m_pages.count() - 1;
    }
    Q_EMIT activePageChanged();
    Q_EMIT countChanged();

    if (!m_changed) {
        m_changed = true;
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
    return m_pages.count();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_pages.size() || index.row() < 0) {
        return QVariant();
    }

    switch (role) {
    case ImageUrlRole:
        if (m_details.at(index.row()).isSaved || m_pages.at(index.row()).temporaryFile.get() != nullptr) {
            return QUrl::fromLocalFile(m_pages.at(index.row()).temporaryFile->fileName());
        } else {
            return QUrl();
        }
    case RotationAngleRole:
        return m_pages.at(index.row()).rotationAngle;
    case IsSavedRole:
        return m_details.at(index.row()).isSaved;
    case AspectRatioRole:
        return m_details.at(index.row()).aspectRatio;
    case PreviewWidthRole:
        return m_details.at(index.row()).previewWidth;
    case PreviewHeightRole:
        return m_details.at(index.row()).previewHeight;
    }
    return QVariant();
}

void DocumentModel::clearData()
{
    beginResetModel();
    m_pages.clear();
    m_details.clear();
    m_activePageIndex = -1;
    endResetModel();
    Q_EMIT countChanged();

    if (defaultFileName != m_name) {
        m_name = defaultFileName;
        Q_EMIT nameChanged();
    }
    if (m_changed) {
        m_changed = false;
        Q_EMIT changedChanged();
    }
}

void DocumentModel::updateFileInformation(const QString &fileName, const SkanpageUtils::DocumentPages &document)
{
    if (document == m_pages && m_changed) {
        m_changed = false;
        Q_EMIT changedChanged();
    }

    if (m_name != fileName) {
        m_name = fileName;
        Q_EMIT nameChanged();
    }
}

