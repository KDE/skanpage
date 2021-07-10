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

#include <KLocalizedString>

#include "skanpage_debug.h"
#include "DocumentSaver.h"
#include "DocumentPrinter.h"

DocumentModel::DocumentModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_name(i18n("New document"))
    , m_documentSaver(std::make_unique<DocumentSaver>())
    , m_documentPrinter(std::make_unique<DocumentPrinter>())
{
    connect(m_documentSaver.get(), &DocumentSaver::showUserMessage, this, &DocumentModel::showUserMessage);
    connect(m_documentSaver.get(), &DocumentSaver::fileSaved, this, &DocumentModel::updateFileInformation);
    connect(m_documentPrinter.get(), &DocumentPrinter::showUserMessage, this, &DocumentModel::showUserMessage);
}

DocumentModel::~DocumentModel()
{
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
    if (m_activePageIndex >= 0 && m_activePageIndex < rowCount()) {
        return QUrl::fromLocalFile(m_pages.at(m_activePageIndex).temporaryFile->fileName());
    }
    return QUrl();
}

void DocumentModel::setActivePageIndex(int newIndex)
{
    if (newIndex != m_activePageIndex) {
        m_activePageIndex = newIndex;
        Q_EMIT activePageChanged();
    }
}

void DocumentModel::save(const QUrl &fileUrl)
{
    m_documentSaver->saveDocument(fileUrl, m_pages);
}

void DocumentModel::print()
{
    m_documentPrinter->printDocument(m_pages);
}

void DocumentModel::addImage(const QImage &image, const int dpi)
{
    const double conversionFactorMM = static_cast<double>(dpi) / 25.4;
    QPageSize pageSize = QPageSize(QSizeF(image.width() / conversionFactorMM, image.height()/ conversionFactorMM), QPageSize::Millimeter);
    QTemporaryFile *tempImageFile = new QTemporaryFile();
    tempImageFile->open();
    if (image.save(tempImageFile, "PNG")) {
        qCDebug(SKANPAGE_LOG) << "Adding new image file" << tempImageFile << " with pageSize" << pageSize << "and resolution " << dpi << "dpi";
    } else {
         Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Failed to save image"));
    }
    tempImageFile->close();
    beginInsertRows(QModelIndex(), m_pages.count(), m_pages.count());
    m_pages.append({std::shared_ptr<QTemporaryFile>(tempImageFile), pageSize, dpi});
    endInsertRows();
   
    Q_EMIT countChanged();
    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    } 
    
    m_activePageIndex = m_pages.count() - 1;
    Q_EMIT activePageChanged();
    Q_EMIT newImageAdded();
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

void DocumentModel::rotateImage(int row, bool positiveDirection)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }
    int rotationAngle = m_pages.at(row).rotationAngle;
    if (positiveDirection) {
        rotationAngle += 90;
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
    Q_EMIT dataChanged(index(row,0), index(row,0), {RotationAngleRole});
}

void DocumentModel::removeImage(int row)
{
    if (row < 0 || row >= m_pages.count()) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), row, row);
    m_pages.removeAt(row);
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
        return QUrl::fromLocalFile(m_pages.at(index.row()).temporaryFile->fileName());
    case RotationAngleRole:
        return m_pages.at(index.row()).rotationAngle;
    }
    return QVariant();
}

void DocumentModel::clearData()
{
    beginResetModel();
    m_pages.clear();
    m_activePageIndex = -1;
    endResetModel();
    Q_EMIT countChanged();

    Q_EMIT countChanged();
    
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

