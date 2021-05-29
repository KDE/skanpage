/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef Skanpage_h
#define Skanpage_h

#include <QMap> // FIXME add this to KSaneWidget!!
#include <QObject>
#include <QPageSize>
#include <QSizeF>
#include <QString>

#include <KSaneWidget>

#include <memory>

class KAboutData;
class DocumentModel;
class DevicesModel;
class OptionsModel;
class SingleOption;

using namespace KSaneIface;

class Skanpage : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
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

    QString errorMessage() const;

    int progress() const;
    ApplicationState applicationState() const;

    DocumentModel *documentModel() const;
    DevicesModel *devicesModel() const;
    OptionsModel *optionsModel() const;
    SingleOption *resolutionOption() const;
    SingleOption *pageSizeOption() const;
    SingleOption *sourceOption() const;
    SingleOption *scanModeOption() const;

    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void reloadDevicesList();
    Q_INVOKABLE bool openDevice(const QString &deviceName);

Q_SIGNALS:
    void progressChanged();
    void optionsChanged();
    void errorMessageChanged();
    void applicationStateChanged();
    void deviceInfoUpdated();

public Q_SLOTS:
    void showAboutDialog();
    void loadScannerOptions();
    void saveScannerOptions();

    void startScan();

private Q_SLOTS:
    void imageReady(const QImage &image);

    void availableDevices(const QList<KSaneWidget::DeviceInfo> &deviceList);

    void alertUser(int type, const QString &strStatus);

    void progressUpdated(int progress);
    void scanDone(int status, const QString &strStatus);

private:
    void finishOpeningDevice(const QString &deviceName);
    void signalErrorMessage(const QString &text);

    std::unique_ptr<KSaneWidget> m_ksanew;
    std::unique_ptr<DocumentModel> m_docHandler;
    std::unique_ptr<DevicesModel> m_availableDevices;
    std::unique_ptr<OptionsModel> m_optionsModel;
    std::unique_ptr<SingleOption> m_resolutionOption;
    std::unique_ptr<SingleOption> m_pageSizeOption;
    std::unique_ptr<SingleOption> m_sourceOption;
    std::unique_ptr<SingleOption> m_scanModeOption;

    int m_progress = 100;
    QString m_errorMessage;
    ApplicationState m_state = NoDeviceOpened;
};

#endif
