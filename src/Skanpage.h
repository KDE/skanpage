/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef SKANPAGE_H
#define SKANPAGE_H

#include <QObject>
#include <QString>

#include <KSaneCore>

#include <memory>

#include "SkanpageUtils.h"

class DocumentModel;
class DevicesModel;
class FormatModel;
class FilteredOptionsModel;
class SkanpagePrivate;

class Skanpage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int countDown READ countDown NOTIFY countDownChanged)
    Q_PROPERTY(DocumentModel *documentModel READ documentModel CONSTANT)
    Q_PROPERTY(DevicesModel *devicesModel READ devicesModel CONSTANT)
    Q_PROPERTY(FormatModel *formatModel READ formatModel CONSTANT)
    Q_PROPERTY(FilteredOptionsModel *optionsModel READ optionsModel CONSTANT)
    Q_PROPERTY(ApplicationState applicationState READ applicationState NOTIFY applicationStateChanged)

    Q_PROPERTY(QString deviceVendor READ deviceVendor NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceModel READ deviceModel NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceInfoUpdated)

public:

    enum ApplicationState {
        ScanInProgress,
        ReadyForScan,
        DeviceSelection,
        SearchingForDevices,
        NoDeviceOpened
    };

    Q_ENUM(ApplicationState);

    explicit Skanpage(const QString &deviceName, QObject *parent = nullptr);
    ~Skanpage();

    QString deviceVendor() const;
    QString deviceModel() const;
    QString deviceName() const;

    int progress() const;
    int countDown() const;
    ApplicationState applicationState() const;

    DocumentModel *documentModel() const;
    DevicesModel *devicesModel() const;
    FormatModel *formatModel() const;
    FilteredOptionsModel *optionsModel() const;
    KSaneIface::KSaneCore *ksaneInterface() const;

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void reloadDevicesList();
    Q_INVOKABLE bool openDevice(const QString &deviceName, const QString &deviceVendor = QString(), const QString &deviceModel = QString());

Q_SIGNALS:
    void progressChanged(int progress);
    void countDownChanged(int remainingSeconds);
    void optionsChanged();
    void applicationStateChanged(ApplicationState state);
    void deviceInfoUpdated();
    void newUserMessage(const QVariant &level, const QVariant &message);

private Q_SLOTS:
    void availableDevices(const QList<KSaneIface::KSaneCore::DeviceInfo> &deviceList);
    void showKSaneMessage(KSaneIface::KSaneCore::KSaneScanStatus type, const QString &strStatus);
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &strStatus);
    void imageReady(const QImage &image);
    void progressUpdated(int progress);
    void batchModeCountDown(int remainingSeconds);
    void scanningFinished(KSaneIface::KSaneCore::KSaneScanStatus statuts, const QString &strStatus);
    void imageTemporarilySaved();

private:
    void finishOpeningDevice(const QString &deviceName, const QString &deviceVendor, const QString &deviceModel);
    void loadScannerOptions();
    void saveScannerOptions();
    void checkFinish();
    void signalErrorMessage(const QString &text);

    std::unique_ptr<SkanpagePrivate> d;
};

#endif // SKANPAGE_H
