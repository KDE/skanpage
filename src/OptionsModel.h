/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef OPTIONS_MODEL_H
#define OPTIONS_MODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QObject>

#include <memory>

#include <KSaneCore/Option>

class OptionsModelPrivate;

class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    enum OptionsModelRoles {
        NameRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        ValueRole,
        MaximumValueRole,
        MinimumValueRole,
        StepValueRole,
        ValueListRole,
        UnitRole,
        TypeRole,
        StateRole,
        QuickAccessRole
    };

    explicit OptionsModel(QObject *parent = nullptr);

    ~OptionsModel() override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = ValueRole) override;

    void setOptionsList(const QList<KSaneCore::Option *> &optionsList);

    void clearOptions();

Q_SIGNALS:

    void rowCountChanged();

private:
    std::unique_ptr<OptionsModelPrivate> d;
};

#endif // OPTIONS_MODEL_H
