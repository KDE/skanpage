/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef Skanpage_h
#define Skanpage_h

#include <QObject>
#include <QString>

#include <KSaneCore>

#include <memory>

#include "SkanpageUtils.h"

class DocumentModel;
class DevicesModel;
class OptionsModel;
class SingleOption;

class Skanpage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int countDown READ countDown NOTIFY countDownChanged)
    Q_PROPERTY(DocumentModel *documentModel READ documentModel CONSTANT)
    Q_PROPERTY(DevicesModel *devicesModel READ devicesModel CONSTANT)
    Q_PROPERTY(OptionsModel *optionsModel READ optionsModel CONSTANT)
    Q_PROPERTY(ApplicationState applicationState READ applicationState NOTIFY applicationStateChanged)

    Q_PROPERTY(QString deviceVendor READ deviceVendor NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceModel READ deviceModel NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceInfoUpdated)

    Q_PROPERTY(SingleOption *resolutionOption READ resolutionOption NOTIFY optionsChanged)
    Q_PROPERTY(SingleOption *pageSizeOption READ pageSizeOption NOTIFY optionsChanged)
    Q_PROPERTY(SingleOption *sourceOption READ sourceOption NOTIFY optionsChanged)
    Q_PROPERTY(SingleOption *scanModeOption READ scanModeOption NOTIFY optionsChanged)

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
    OptionsModel *optionsModel() const;
    KSaneIface::KSaneCore *ksaneInterface() const;
    SingleOption *resolutionOption() const;
    SingleOption *pageSizeOption() const;
    SingleOption *sourceOption() const;
    SingleOption *scanModeOption() const;

    Q_INVOKABLE void startScan();
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void reloadDevicesList();
    Q_INVOKABLE bool openDevice(const QString &deviceName);

Q_SIGNALS:
    void progressChanged(int progress);
    void countDownChanged(int remainingSeconds);
    void optionsChanged();
    void applicationStateChanged();
    void deviceInfoUpdated();
    void newUserMessage(QVariant level, const QVariant &message);

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
    void finishOpeningDevice(const QString &deviceName);
    void loadScannerOptions();
    void saveScannerOptions();
    void checkFinish();
    void signalErrorMessage(const QString &text);

    std::unique_ptr<KSaneIface::KSaneCore> m_ksaneInterface;
    std::unique_ptr<DocumentModel> m_docHandler;
    std::unique_ptr<DevicesModel> m_availableDevices;
    std::unique_ptr<OptionsModel> m_optionsModel;
    std::unique_ptr<SingleOption> m_resolutionOption;
    std::unique_ptr<SingleOption> m_pageSizeOption;
    std::unique_ptr<SingleOption> m_sourceOption;
    std::unique_ptr<SingleOption> m_scanModeOption;

    int m_progress = 100;
    int m_remainingSeconds = 0;
    int m_scannedImages = 0;
    ApplicationState m_state = NoDeviceOpened;
    bool m_scanInProgress = false;
};

#endif
