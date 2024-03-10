/*
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FormatModel.h"

#include <QImageWriter>
#include <QImageReader>
#include <QMimeType>
#include <QMimeDatabase>

#include <KLocalizedString>

class FormatModelPrivate
{
public:
    QList<QMimeType> m_writeFormatList;
    QVariantList m_writeFormatFilter;
    QVariantList m_readFormatFilter;
};

FormatModel::FormatModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::make_unique<FormatModelPrivate>())
{
    const QMimeDatabase mimeDB;
    QList<QByteArray> tempWriteList = QImageWriter::supportedMimeTypes();
    // Put first class citizens at first place
    tempWriteList.removeAll(QByteArray("image/jpeg"));
    tempWriteList.removeAll(QByteArray("image/png"));
    int count = tempWriteList.removeAll(QByteArray("image/tiff")); // TIFF is not on the base list of formats
    tempWriteList.insert(0, QByteArray("application/pdf"));
    tempWriteList.insert(1, QByteArray("image/png"));
    tempWriteList.insert(2, QByteArray("image/jpeg"));
    if (count > 0) {
        tempWriteList.insert(3, QByteArray("image/tiff"));
    }

    for (const auto &mimeString : std::as_const(tempWriteList)) {
        const QMimeType mimeType = mimeDB.mimeTypeForName(QString::fromLatin1(mimeString));
        d->m_writeFormatList.append(mimeType);
        // craft a string that QML's FileDialog understands
        d->m_writeFormatFilter.append({mimeType.comment() + QStringLiteral("(*.") + mimeType.preferredSuffix() + QStringLiteral(")")});
    }
    QList<QByteArray> tempReadList = QImageReader::supportedMimeTypes();
    tempReadList.removeAll(QByteArray("image/jpeg"));
    tempReadList.removeAll(QByteArray("image/tiff"));
    tempReadList.removeAll(QByteArray("image/png"));
    tempReadList.insert(0, QByteArray("image/png"));
    tempReadList.insert(1, QByteArray("image/jpeg"));
    tempReadList.insert(2, QByteArray("image/tiff"));
    d->m_readFormatFilter.append({i18n("All files") + QStringLiteral(" (*)")});
    for (const auto &mimeString : std::as_const(tempReadList)) {
        const QMimeType mimeType = mimeDB.mimeTypeForName(QString::fromLatin1(mimeString));
        // craft a string that QML's FileDialog understands
        const auto &suffixes = mimeType.suffixes();
        QString filter = QStringLiteral("(");
        for (const auto &suffix : suffixes) {
            filter.append(QStringLiteral("*.") + suffix + QStringLiteral(" "));
        }
        filter.append(QStringLiteral(")"));
        d->m_readFormatFilter.append({mimeType.comment() + filter});
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
    roles[NameFilterRole] = "nameFilter";
    return roles;
}

int FormatModel::rowCount(const QModelIndex &) const
{
    return d->m_writeFormatList.count();
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
    if (index >= d->m_writeFormatList.size() || index < 0) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return d->m_writeFormatList.at(index).name();
        break;
    case SuffixRole:
        return d->m_writeFormatList.at(index).preferredSuffix();
        break;
    case CommentRole:
        return d->m_writeFormatList.at(index).comment();
        break;
    case NameFilterRole:
        return d->m_writeFormatFilter.at(index);
        break;
    default:
        break;
    }
    return QVariant();
}

QVariantList FormatModel::writeFormatFilter() const
{
    return d->m_writeFormatFilter;
}

QVariantList FormatModel::importFormatFilter() const
{
    return d->m_readFormatFilter;
}

QString FormatModel::pdfFormatFilter() const
{
    return d->m_writeFormatFilter.at(0).toString();
}

#include "moc_FormatModel.cpp"
