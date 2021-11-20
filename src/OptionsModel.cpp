/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

//KDE includes
#include <KLocalizedString>

#include "OptionsModel.h"
#include "skanpage_debug.h"

class OptionsModelPrivate
{
public:
    QList<KSaneIface::KSaneOption *> mOptionsList;
    QList<bool> mFilterList;
};

OptionsModel::OptionsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::make_unique<OptionsModelPrivate>())
{
}

OptionsModel::~OptionsModel()
{
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
    roles[FilterRole] = "filter";
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
    case FilterRole:
        return d->mFilterList.at(index.row());
        break;
    default:
        break;
    }
    return QVariant();
}

bool OptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((role != ValueRole && role != FilterRole) || index.row() < 0 || index.row() >= d->mOptionsList.size()) {
        return false;
    }
    if (role == ValueRole) {
        d->mOptionsList.at(index.row())->setValue(value);
        qCDebug(SKANPAGE_LOG()) << "OptionsModel: Writing to option" << d->mOptionsList.at(index.row())->name() << value;
        Q_EMIT dataChanged(index, index, {ValueRole});
        return true;
    }
    if (role == FilterRole) {
        d->mFilterList[index.row()] = value.toBool();
        Q_EMIT dataChanged(index, index, {FilterRole});
    }
    return true;
}

void OptionsModel::setOptionsList(const QList<KSaneIface::KSaneOption *> optionsList)
{
    beginResetModel();
    d->mOptionsList = optionsList;
    d->mFilterList.clear();
    d->mFilterList.reserve(optionsList.size());
    for (int i = 0; i < d->mOptionsList.size(); i++) {
        KSaneIface::KSaneOption *option = d->mOptionsList.at(i);
        qCDebug(SKANPAGE_LOG()) << "OptionsModel: Importing option " << option->name() << ", type" << option->type() << ", state" << option->state();
        connect(option, &KSaneIface::KSaneOption::optionReloaded, this, [=]() { Q_EMIT dataChanged(index(i, 0), index(i, 0), {StateRole}); });
        connect(option, &KSaneIface::KSaneOption::valueChanged, this, [=]() { Q_EMIT dataChanged(index(i, 0), index(i, 0), {ValueRole}); });
        if (option->name() == QStringLiteral("KSane::PageSize") || option->name() == QStringLiteral("resolution") || option->name() == QStringLiteral("source") || option->name() == QStringLiteral("mode") ) {
            d->mFilterList.insert(i, true);
        } else {
            d->mFilterList.insert(i, false);
        }
    }
    endResetModel();
    Q_EMIT rowCountChanged();
}

void OptionsModel::clearOptions()
{
    beginResetModel();
    d->mOptionsList.clear();
    d->mFilterList.clear();
    endResetModel();
    Q_EMIT rowCountChanged();
}
