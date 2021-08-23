/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

//KDE includes

#include <klocalizedstring.h>

#include "OptionsModel.h"
#include "skanpage_debug.h"

class OptionsModelPrivate
{
public:
    QList<KSaneIface::KSaneOption *> mOptionsList;
    QVariantList mCurrentValueList;
    bool mIsModified = false;
};

OptionsModel::OptionsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::unique_ptr<OptionsModelPrivate>(new OptionsModelPrivate()))
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
    roles[VisibleRole] = "visible";
    return roles;
}

int OptionsModel::rowCount(const QModelIndex &) const
{
    return d->mOptionsList.count();
}

bool OptionsModel::isModified() const
{
    return d->mIsModified;
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
        return d->mCurrentValueList.at(index.row());
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
    case VisibleRole:
        return d->mOptionsList.at(index.row())->state() == KSaneIface::KSaneOption::KSaneOptionState::StateActive;
    default:
        break;
    }
    return QVariant();
}

bool OptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != ValueRole || index.row() < 0 || index.row() >= d->mOptionsList.size()) {
        return false;
    }
    if (d->mOptionsList.at(index.row())->type() == KSaneIface::KSaneOption::TypeAction) {
        d->mOptionsList.at(index.row())->setValue(value);
    } else {
        d->mCurrentValueList[index.row()] = value;
        d->mIsModified = true;
        Q_EMIT isModifiedChanged();
    }
    return true;
}

void OptionsModel::setOptionsList(const QList<KSaneIface::KSaneOption *> optionsList)
{
    beginResetModel();
    d->mOptionsList = optionsList;
    d->mCurrentValueList.clear();
    d->mCurrentValueList.reserve(optionsList.size());
    for (int i = 0; i < d->mOptionsList.size(); i++) {
        KSaneIface::KSaneOption *option = d->mOptionsList.at(i);
        qCDebug(SKANPAGE_LOG()) << "OptionsModel: Importing option " << option->name() << ", type" << option->type() << ", state" << option->state();
        d->mCurrentValueList.append(option->value());
        connect(option, &KSaneIface::KSaneOption::optionReloaded, this, [=]() { Q_EMIT dataChanged(index(i, 0), index(i, 0), {VisibleRole}); });
        connect(option, &KSaneIface::KSaneOption::valueChanged, this, [=]() { d->mCurrentValueList[i] = option->value(); Q_EMIT dataChanged(index(i, 0), index(i, 0), {ValueRole}); });
    }
    endResetModel();
    Q_EMIT rowCountChanged();
}

void OptionsModel::resetOptionsValues()
{
    if (d->mIsModified == false) {
        return;
    }
    for (int i = 0; i < d->mOptionsList.size(); i++) {
        if (d->mCurrentValueList[i] != d->mOptionsList.at(i)->value()) {;
            d->mCurrentValueList[i] = d->mOptionsList.at(i)->value();
            Q_EMIT dataChanged(index(i, 0), index(i, 0), {ValueRole});
        }
    }
    d->mIsModified = false;
    Q_EMIT isModifiedChanged();
    qCDebug(SKANPAGE_LOG()) << "OptionsModel reset";
}

void OptionsModel::saveOptionsValues()
{
    if (d->mIsModified == false) {
        return;
    }
    for (int i = 0; i < d->mOptionsList.size(); i++) {
        if (d->mOptionsList.at(i)->state() == KSaneIface::KSaneOption::KSaneOptionState::StateActive &&
            d->mOptionsList.at(i)->type() != KSaneIface::KSaneOption::TypeAction &&
            d->mOptionsList.at(i)->value() != d->mCurrentValueList.at(i)) {

            d->mOptionsList.at(i)->setValue(d->mCurrentValueList.at(i));
            qCDebug(SKANPAGE_LOG()) << "OptionsModel: Writing to option" << d->mOptionsList.at(i)->name() << d->mCurrentValueList.at(i);
        }
    }
    d->mIsModified = false;
    Q_EMIT isModifiedChanged();
}

void OptionsModel::clearOptions()
{
    beginResetModel();
    d->mOptionsList.clear();
    d->mCurrentValueList.clear();
    d->mIsModified = false;
    endResetModel();
    Q_EMIT rowCountChanged();
    Q_EMIT isModifiedChanged();
}
