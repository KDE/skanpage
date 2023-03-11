/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

//KDE includes
#include <KConfigGroup>
#include <KSharedConfig>

#include "OptionsModel.h"
#include "skanpage_debug.h"

class OptionsModelPrivate
{
public:
    QList<KSaneCore::Option *> mOptionsList;
    QList<bool> mQuickAccessList;
    QSet<QString> mQuickAccessOptions;
    bool mQuickAccessListChanged = false;
};

OptionsModel::OptionsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::make_unique<OptionsModelPrivate>())
{
    //if there are no defined quick access options yet, insert default ones
    KConfigGroup quickOptions(KSharedConfig::openConfig(), QStringLiteral("quickAccessOptions"));
    if (!quickOptions.exists()) {
        d->mQuickAccessOptions.insert(QStringLiteral("KSane::PageSize"));
        d->mQuickAccessOptions.insert(QStringLiteral("resolution"));
        d->mQuickAccessOptions.insert(QStringLiteral("source"));
        d->mQuickAccessOptions.insert(QStringLiteral("mode"));
    } else {
        const QStringList keys = quickOptions.keyList();
        d->mQuickAccessOptions = QSet(keys.begin(), keys.end());
    }
}

OptionsModel::~OptionsModel()
{    
    if (d->mQuickAccessListChanged) {
        KConfigGroup quickOptions(KSharedConfig::openConfig(), QStringLiteral("quickAccessOptions"));
        quickOptions.deleteGroup();
        for (int i = 0; i < d->mOptionsList.size(); i++) {
            if (d->mQuickAccessList.at(i)) {
                quickOptions.writeEntry(d->mOptionsList.at(i)->name(), true);
            }
        }
    }
}

QHash<int, QByteArray> OptionsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[TitleRole] = "title";
    roles[DescriptionRole] = "description";
    roles[ValueRole] = "value";
    roles[MaximumValueRole] = "maximum";
    roles[MinimumValueRole] = "minimum";
    roles[StepValueRole] = "step";
    roles[ValueListRole] = "valueList";
    roles[UnitRole] = "unit";
    roles[TypeRole] = "type";
    roles[StateRole] = "state";
    roles[QuickAccessRole] = "quickAccess";
    return roles;
}

int OptionsModel::rowCount(const QModelIndex &) const
{
    return d->mOptionsList.count();
}

QVariant OptionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= d->mOptionsList.size() || index.row() < 0) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return d->mOptionsList.at(index.row())->name();
        break;
    case TitleRole:
        return d->mOptionsList.at(index.row())->title();
        break;
    case DescriptionRole:
        return d->mOptionsList.at(index.row())->description();
        break;
    case ValueRole:
        return d->mOptionsList.at(index.row())->value();
        break;
    case MaximumValueRole:
        return d->mOptionsList.at(index.row())->maximumValue();
        break;
    case MinimumValueRole:
        return d->mOptionsList.at(index.row())->minimumValue();
        break;
    case StepValueRole:
        return d->mOptionsList.at(index.row())->stepValue();
        break;
    case ValueListRole:
        return d->mOptionsList.at(index.row())->valueList();
        break;
    case UnitRole:
        return d->mOptionsList.at(index.row())->valueUnit();
        break;
    case TypeRole:
        return d->mOptionsList.at(index.row())->type();
        break;
    case StateRole:
        return d->mOptionsList.at(index.row())->state();
        break;
    case QuickAccessRole:
        return d->mQuickAccessList.at(index.row());
        break;
    default:
        break;
    }
    return QVariant();
}

bool OptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((role != ValueRole && role != QuickAccessRole) || index.row() < 0 || index.row() >= d->mOptionsList.size()) {
        return false;
    }
    if (role == ValueRole) {
        qCDebug(SKANPAGE_LOG()) << "OptionsModel: Writing to option" << d->mOptionsList.at(index.row())->name() << value;
        d->mOptionsList.at(index.row())->setValue(value);
        Q_EMIT dataChanged(index, index, {ValueRole});
        return true;
    }
    if (role == QuickAccessRole) {
        d->mQuickAccessList[index.row()] = value.toBool();
        d->mQuickAccessListChanged = true;
        Q_EMIT dataChanged(index, index, {QuickAccessRole});
    }
    return true;
}

void OptionsModel::setOptionsList(const QList<KSaneCore::Option *> &optionsList)
{
    beginResetModel();
    d->mOptionsList = optionsList;
    d->mQuickAccessList.clear();
    d->mQuickAccessList.reserve(optionsList.size());
    for (int i = 0; i < d->mOptionsList.size(); i++) {
        KSaneCore::Option *option = d->mOptionsList.at(i);
        qCDebug(SKANPAGE_LOG()) << "OptionsModel: Importing option " << option->name() << ", type" << option->type() << ", state" << option->state();
        connect(option, &KSaneCore::Option::optionReloaded, this, [=]() { Q_EMIT dataChanged(index(i, 0), index(i, 0), {StateRole}); });
        connect(option, &KSaneCore::Option::valueChanged, this, [=]() { Q_EMIT dataChanged(index(i, 0), index(i, 0), {ValueRole}); });
        d->mQuickAccessList.insert(i, d->mQuickAccessOptions.contains(option->name()));
    }
    endResetModel();
    Q_EMIT rowCountChanged();
}

void OptionsModel::clearOptions()
{
    beginResetModel();
    d->mOptionsList.clear();
    d->mQuickAccessList.clear();
    endResetModel();
    Q_EMIT rowCountChanged();
}
