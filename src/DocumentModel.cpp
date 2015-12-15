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
#include <QPdfWriter>
#include <QPainter>

#include <KLocalizedString>

DocumentModel::DocumentModel(QObject *parent) : QAbstractListModel(parent)
, m_name(i18n("Unnamed"))
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

void DocumentModel::addImage(QTemporaryFile *tmpFile)
{
    if (tmpFile == nullptr) {
        qDebug() << tmpFile;
        return;
    }

    beginInsertRows(QModelIndex(), m_tmpFiles.count(), m_tmpFiles.count());
    m_tmpFiles.append(tmpFile);
    endInsertRows();
    if (!m_changed) {
        m_changed = true;
        emit changedChanged();
    }
}

void DocumentModel::save(const QString &name, const QSizeF &pageSize, int dpi, const QString &title)
{
    QPdfWriter writer(name);

    writer.setPageSize(QPageSize(pageSize, QPageSize::Millimeter));
    writer.setResolution(dpi);
    writer.setPageMargins(QMarginsF(0,0,0,0));
    writer.setCreator(QStringLiteral("Skanpage"));
    writer.setTitle(title);


    QPainter painter(&writer);
    for (int i=0; i<m_tmpFiles.count(); ++i) {
        QRect target(0, 0, writer.width(), writer.height());
        QPixmap pixmap(m_tmpFiles[i]->fileName());
        pixmap = pixmap.scaled(target.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawPixmap(pixmap.rect(), pixmap, pixmap.rect());
        if (i<m_tmpFiles.count()-1) {
            writer.newPage();
        }
    }
    painter.end();

    if (m_changed) {
        m_changed = false;
        emit changedChanged();
    }
}

void DocumentModel::moveImage(int from, int to)
{
    int add = 0;
    if (from == to) return;
    if (to > from) {add = 1;}
    if (from < 0 || from >= m_tmpFiles.count()) return;
    if (to < 0 || to >= m_tmpFiles.count()) return;
    bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), to+add);
    if (!ok) {
        qDebug() << "Failed to move" << from << to << add << m_tmpFiles.count();
        return;
    }
    m_tmpFiles.move(from, to);
    endMoveRows();
}

void DocumentModel::removeImage(int row)
{
    if (row < 0 || row >= m_tmpFiles.count()) {
        return;
    }

    beginRemoveRows(QModelIndex() , row, row);
    m_tmpFiles.removeAt(row);
    endRemoveRows();
}

QHash<int, QByteArray> DocumentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[FileNameRole] = "name";
    return roles;
}


int DocumentModel::rowCount(const QModelIndex &) const
{
    return m_tmpFiles.count();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_tmpFiles.size() || index.row() < 0) {
        return QVariant();
    }

    switch (role) {
        case FileNameRole:
            return m_tmpFiles[index.row()]->fileName();
    }
    return QVariant();
}


