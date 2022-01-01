/*
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FormatModel.h"

#include <QImageWriter>
#include <QMimeType>
#include <QMimeDatabase>

#include <KLocalizedString>

class FormatModelPrivate
{
public:
    QList<QMimeType> m_formatList;
    QVariantList m_formatFilter;
};

FormatModel::FormatModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::unique_ptr<FormatModelPrivate>(new FormatModelPrivate()))
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

    for (const auto &mimeString : tempList) {
        const QMimeType mimeType = mimeDB.mimeTypeForName(QString::fromLatin1(mimeString));
        d->m_formatList.append(mimeType);
        // craft a string that QML's FileDialog understands
        d->m_formatFilter.append({mimeType.comment() + QStringLiteral("(*.") + mimeType.preferredSuffix() + QStringLiteral(")")});
    }
}

FormatModel::~FormatModel()
{
}

QHash<int, QByteArray> FormatModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[SuffixRole] = "suffix";
    roles[CommentRole] = "comment";
    return roles;
}

int FormatModel::rowCount(const QModelIndex &) const
{
    return d->m_formatList.count();
}

QVariant FormatModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return getData(index.row(), role);
}

QVariant FormatModel::getData(int index, int role) const
{
    if (index >= d->m_formatList.size() || index < 0) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return d->m_formatList.at(index).name();
        break;
    case SuffixRole:
        return d->m_formatList.at(index).preferredSuffix();
        break;
    case CommentRole:
        return d->m_formatList.at(index).comment();
        break;
    default:
        break;
    }
    return QVariant();
}

QVariantList FormatModel::formatFilter() const
{
    return d->m_formatFilter;
}
