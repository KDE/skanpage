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
#ifndef DEVICESMODEL_H
#define DEVICESMODEL_H

#include <QObject>
#include <QList>
#include <QAbstractListModel>
#include <QDebug>

#include <memory>

#include <KSaneWidget>

class DevicesModelPrivate;

using namespace KSaneIface;

class DevicesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    
    enum DevicesModelRoles {
        NameRole = Qt::UserRole+1,
        VendorRole,
        ModelRole,
        TypeRole
    };
    
    explicit DevicesModel(QObject *parent = nullptr);
 
    ~DevicesModel();

public:
    
    QHash<int, QByteArray> roleNames() const override;
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    
    Q_INVOKABLE QString getSelectedDeviceName() const;

    void updateDevicesList(QList<KSaneWidget::DeviceInfo> deviceList);
    
public Q_SLOTS:
    
    void selectDevice(int i);

private:
    
    std::unique_ptr<DevicesModelPrivate> d;
};

#endif // DEVICESMODEL_H
