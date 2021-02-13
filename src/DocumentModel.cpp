/* ============================================================
 *
 * Copyright (C) 2015 by Kåre Särs <kare.sars@iki .fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * ============================================================ */
#include "DocumentModel.h"

#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QPdfWriter>
#include <QUrl>

#include "skanpage_debug.h"
#include <KLocalizedString>

DocumentModel::DocumentModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_name(i18n("New document"))
    , m_changed(false)
{
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

void DocumentModel::save(const QUrl &fileUrl)
{
    if (fileUrl.isEmpty() || m_pages.isEmpty()) {
        return;
    }
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Saving document to") << fileUrl;

    const auto localFile = fileUrl.toLocalFile();
    const auto fileInfo = QFileInfo(localFile);
    const auto fileSuffix = fileInfo.suffix();

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Selected file suffix is") << fileSuffix;

    if (fileSuffix == QLatin1String("pdf") || fileSuffix.isEmpty()) {
        savePDF(localFile);
    } else {
        saveImage(fileInfo);
    }

    if (m_changed) {
        m_changed = false;
        Q_EMIT changedChanged();
    }

    if (m_name != fileInfo.fileName()) {
        m_name = fileInfo.fileName();
        Q_EMIT nameChanged();
    }
}

void DocumentModel::savePDF(const QString &name)
{
    QPdfWriter writer(name);

    writer.setResolution(m_pages.at(0).dpi);
    writer.setPageSize(QPageSize(m_pages.at(0).pageSize));
    writer.setPageMargins(QMarginsF(0, 0, 0, 0));

    QPainter painter(&writer);
    for (int i = 0; i < m_pages.count(); ++i) {
        QSize targetSize( writer.width(), writer.height());
        QImage image(m_pages.at(i).tmpFile->fileName());
        image = image.scaled(targetSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawImage(image.rect(), image, image.rect());
        if (i < m_pages.count() - 1) {
            writer.setPageMargins(QMarginsF(0, 0, 0, 0));
            writer.setResolution(m_pages.at(i + 1).dpi);
            writer.setPageSize(QPageSize(m_pages.at(i + 1).pageSize));
            writer.newPage();
        }
    }
    painter.end();
}

void DocumentModel::saveImage(const QFileInfo &fileInfo)
{
    const int count = m_pages.count();
    QImage image;
    QString fileName;

    if (count == 1) {
        image.load(m_pages.at(0).tmpFile->fileName());
        fileName = fileInfo.absoluteFilePath();
        image.save(fileName, fileInfo.suffix().toLocal8Bit().constData());
    } else {
        for (int i = 0; i < count; ++i) {
            image.load(m_pages.at(i).tmpFile->fileName());
            fileName =
                QStringLiteral("%1/%2%3.%4").arg(fileInfo.absolutePath()).arg(fileInfo.baseName()).arg(i, 4, 10, QLatin1Char('0')).arg(fileInfo.suffix());
            image.save(fileName, fileInfo.suffix().toLocal8Bit().constData());
        }
    }
}

void DocumentModel::addImage(QTemporaryFile *tmpFile, QPageSize pageSize, int dpi)
{
    if (tmpFile == nullptr) {
        qCDebug(SKANPAGE_LOG) << tmpFile;
        return;
    }

    beginInsertRows(QModelIndex(), m_pages.count(), m_pages.count());
    m_pages.append({tmpFile, pageSize, dpi});
    endInsertRows();
    Q_EMIT countChanged();
    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }
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

    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }
}

void DocumentModel::removeImage(int row)
{
    if (row < 0 || row >= m_pages.count()) {
        return;
    }

    beginRemoveRows(QModelIndex(), row, row);
    m_pages.removeAt(row);
    endRemoveRows();
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
        return QUrl::fromLocalFile(m_pages[index.row()].tmpFile->fileName());
    }
    return QVariant();
}

void DocumentModel::clearData()
{
    beginResetModel();
    m_pages.clear();
    endResetModel();

    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }
}
