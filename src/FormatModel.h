/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef FORMAT_MODEL_H
#define FORMAT_MODEL_H

#include <QAbstractListModel>

#include <memory>

class FormatModelPrivate;

class FormatModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum FormatModelRoles { NameRole = Qt::UserRole + 1, SuffixRole, CommentRole, NameFilterRole };

    Q_ENUM(FormatModelRoles)

    explicit FormatModel(QObject *parent = nullptr);

    ~FormatModel() override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QVariant getData(int index, int role) const;

    Q_INVOKABLE QVariantList writeFormatFilter() const;

    Q_INVOKABLE QString pdfFormatFilter() const;

    Q_INVOKABLE QVariant readFormatFilterConcatenated() const;

private:
    std::unique_ptr<FormatModelPrivate> d;
};

#endif // FORMAT_MODEL_H
