/**
 * SPDX-FileCopyrightText: 2020 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef DEVICES_MODEL_H
#define DEVICES_MODEL_H

#include <QAbstractListModel>
#include <QDebug>
#include <QList>
#include <QObject>

#include <memory>

#include <KSaneCore>

class DevicesModelPrivate;

using namespace KSaneIface;

class DevicesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    enum DevicesModelRoles { NameRole = Qt::UserRole + 1, VendorRole, ModelRole, TypeRole };

    explicit DevicesModel(QObject *parent = nullptr);

    ~DevicesModel();

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QString getSelectedDeviceName() const;

    void updateDevicesList(QList<KSaneCore::DeviceInfo> deviceList);

public Q_SLOTS:

    void selectDevice(int i);

Q_SIGNALS:

    void rowCountChanged();

private:
    std::unique_ptr<DevicesModelPrivate> d;
};

QDebug operator<<(QDebug d, const KSaneCore::DeviceInfo &deviceInfo);

#endif // DEVICES_MODEL_H
