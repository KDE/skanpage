/*
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FormatModel.h"

#include <QImageReader>
#include <QImageWriter>
#include <QMimeDatabase>
#include <QMimeType>

#include <KLocalizedString>

class FormatModelPrivate
{
public:
    QList<QMimeType> m_writeFormatList;
    QVariantList m_writeFormatFilter;
    QVariant m_readFormatFilterConcatenated;
};

FormatModel::FormatModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::make_unique<FormatModelPrivate>())
{
    const QMimeDatabase mimeDB;
    QList<QByteArray> tempList;
    tempList = QImageWriter::supportedMimeTypes();
    // Put first class citizens at first place
    tempList.removeAll(QByteArray("image/jpeg"));
    tempList.removeAll(QByteArray("image/png"));
    tempList.removeAll(QByteArray("application/pdf"));

    tempList.insert(0, QByteArray("application/pdf"));
    tempList.insert(1, QByteArray("image/png"));
    tempList.insert(2, QByteArray("image/jpeg"));

    for (const auto &mimeString : std::as_const(tempList)) {
        const QMimeType mimeType = mimeDB.mimeTypeForName(QString::fromLatin1(mimeString));
        d->m_writeFormatList.append(mimeType);
        // craft a string that QML's FileDialog understands
        d->m_writeFormatFilter.append({mimeType.comment() + QStringLiteral("(*.") + mimeType.preferredSuffix() + QStringLiteral(")")});
    }

    // create a concatenated read filter for import dialog
    tempList = QImageReader::supportedMimeTypes();
    QString readFormatFilter = i18nc("format filter for a file dialog, all formats supported by the application will be displayed", "All Supported Files");
    readFormatFilter.append(QStringLiteral(" ("));
    for (const auto &mimeString : std::as_const(tempList)) {
        const QMimeType mimeType = mimeDB.mimeTypeForName(QString::fromLatin1(mimeString));
        readFormatFilter.append(QStringLiteral(" *.") + mimeType.preferredSuffix());
    }
    readFormatFilter.append(QStringLiteral(" )"));
    d->m_readFormatFilterConcatenated = readFormatFilter;
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

QString FormatModel::pdfFormatFilter() const
{
    return d->m_writeFormatFilter.at(0).toString();
}

QVariant FormatModel::readFormatFilterConcatenated() const
{
    return d->m_readFormatFilterConcatenated;
}

#include "moc_FormatModel.cpp"
