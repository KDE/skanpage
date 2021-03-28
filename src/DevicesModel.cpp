/**
 * SPDX-FileCopyrightText: 2020 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DevicesModel.h"

class DevicesModelPrivate
{
public:
    QList<KSaneWidget::DeviceInfo> mDeviceslist;

    int mSelectedDevice = 0;
};

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(std::unique_ptr<DevicesModelPrivate>(new DevicesModelPrivate()))
{
}

DevicesModel::~DevicesModel()
{
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[VendorRole] = "vendor";
    roles[ModelRole] = "model";
    roles[TypeRole] = "type";
    return roles;
}

int DevicesModel::rowCount(const QModelIndex &) const
{
    return d->mDeviceslist.count();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= d->mDeviceslist.size() || index.row() < 0) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return d->mDeviceslist[index.row()].name;
        break;
    case VendorRole:
        return d->mDeviceslist[index.row()].vendor;
        break;
    case ModelRole:
        return d->mDeviceslist[index.row()].model;
        break;
    case TypeRole:
        return d->mDeviceslist[index.row()].type;
        break;
    default:
        break;
    }
    return QVariant();
}

void DevicesModel::updateDevicesList(QList<KSaneWidget::DeviceInfo> deviceList)
{
    beginResetModel();
    d->mDeviceslist = deviceList;
    endResetModel();
    Q_EMIT rowCountChanged();
}

QString DevicesModel::getSelectedDeviceName() const
{
    if (d->mSelectedDevice >= 0 && d->mSelectedDevice < d->mDeviceslist.count()) {
        return d->mDeviceslist.at(d->mSelectedDevice).name;
    }
    return QString();
}

void DevicesModel::selectDevice(int i)
{
    d->mSelectedDevice = i;
}

QDebug operator<<(QDebug d, const KSaneWidget::DeviceInfo &deviceInfo)
{
    d << "Device name: " << deviceInfo.name << "\n";
    d << "Device vendor: " << deviceInfo.vendor << "\n";
    d << "Device model: " << deviceInfo.model << "\n";
    d << "Device type: " << deviceInfo.type << "\n";
    return d;
}
