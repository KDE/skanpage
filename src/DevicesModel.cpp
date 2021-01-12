/* ============================================================
 *
 * Copyright (C) 2020 by Alexander Stippich <a.stippich@gmx.net>
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

#include "DevicesModel.h"

// Sane includes
extern "C"
{
#include <sane/saneopts.h>
#include <sane/sane.h>
}

class DevicesModelPrivate
{
public:
    
    QList<KSaneWidget::DeviceInfo> mDeviceslist;
    
    int mSelectedDevice = 0;
};

DevicesModel::DevicesModel(QObject *parent) : QAbstractListModel(parent), d(std::unique_ptr<DevicesModelPrivate>(new DevicesModelPrivate()))
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

QDebug operator<< (QDebug d, const KSaneWidget::DeviceInfo &deviceInfo) {
    d << "Device name: " << deviceInfo.name << "\n";
    d << "Device vendor: " << deviceInfo.vendor << "\n";
    d << "Device model: " << deviceInfo.model << "\n";
    d << "Device type: " << deviceInfo.type << "\n";
    return d;
}
