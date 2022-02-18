/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Skanpage.h"

#include <QThread>

#include <KAboutData>
#include <KConfigGroup>
#include <KSharedConfig>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "OptionsModel.h"
#include "FormatModel.h"
#include "FilteredOptionsModel.h"
#include "DocumentSaver.h"
#include "DocumentPrinter.h"
#include "skanpage_debug.h"

class SkanpagePrivate {
public:
    KSaneIface::KSaneCore m_ksaneInterface;
    DocumentModel m_documentHandler;
    DevicesModel m_availableDevices;
    OptionsModel m_optionsModel;
    FormatModel m_formatModel;
    FilteredOptionsModel m_filteredOptionsModel;
    DocumentSaver m_documentSaver;
    DocumentPrinter m_documentPrinter;
    QThread m_fileIOThread;

    int m_progress = 100;
    int m_remainingSeconds = 0;
    int m_scannedImages = 0;
    Skanpage::ApplicationState m_state = Skanpage::SearchingForDevices;
    bool m_scanInProgress = false;
    QString m_deviceName;
    QString m_deviceVendor;
    QString m_deviceModel;
};

Skanpage::Skanpage(const QString &deviceName, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<SkanpagePrivate>())
{
    d->m_filteredOptionsModel.setSourceModel(&d->m_optionsModel);

    connect(&d->m_ksaneInterface, &KSaneCore::scannedImageReady, this, &Skanpage::imageReady);
    connect(&d->m_ksaneInterface, &KSaneCore::availableDevices, this, &Skanpage::availableDevices);
    connect(&d->m_ksaneInterface, &KSaneCore::userMessage, this, &Skanpage::showKSaneMessage);
    connect(&d->m_ksaneInterface, &KSaneCore::scanProgress, this, &Skanpage::progressUpdated);
    connect(&d->m_ksaneInterface, &KSaneCore::scanFinished, this, &Skanpage::scanningFinished);
    connect(&d->m_ksaneInterface, &KSaneCore::batchModeCountDown, this, &Skanpage::batchModeCountDown);
    connect(&d->m_documentHandler, &DocumentModel::newPageAdded, this, &Skanpage::imageTemporarilySaved);
    connect(&d->m_documentHandler, &DocumentModel::printDocument, &d->m_documentPrinter, &DocumentPrinter::printDocument);
    
    d->m_fileIOThread.start();
    d->m_documentSaver.moveToThread(&d->m_fileIOThread);

    connect(&d->m_documentHandler, &DocumentModel::saveDocument, &d->m_documentSaver, &DocumentSaver::saveDocument);
    connect(&d->m_documentHandler, &DocumentModel::saveNewPageTemporary, &d->m_documentSaver, &DocumentSaver::saveNewPageTemporary);
    connect(&d->m_documentSaver, &DocumentSaver::pageTemporarilySaved, &d->m_documentHandler, &DocumentModel::updatePageInModel);
    connect(&d->m_documentSaver, &DocumentSaver::showUserMessage, this, &Skanpage::showUserMessage);
    connect(&d->m_documentSaver, &DocumentSaver::fileSaved, &d->m_documentHandler, &DocumentModel::updateFileInformation);
    connect(&d->m_documentSaver, &DocumentSaver::sharingFileSaved, &d->m_documentHandler, &DocumentModel::updateSharingFileInformation);
    connect(&d->m_documentPrinter, &DocumentPrinter::showUserMessage, this, &Skanpage::showUserMessage);

    // try to open device from command line option first, then remembered device
    if (deviceName.isEmpty() || !openDevice(deviceName)) {

        KConfigGroup options(KSharedConfig::openStateConfig(), QStringLiteral("general"));
        const QString savedDeviceName = options.readEntry(QStringLiteral("deviceName"));
        const QString savedDeviceVendor = options.readEntry(QStringLiteral("deviceVendor"));
        const QString savedDeviceModel = options.readEntry(QStringLiteral("deviceModel"));

        if (!openDevice(savedDeviceName, savedDeviceVendor, savedDeviceModel)) {
            reloadDevicesList();
        }
    }
}

Skanpage::~Skanpage()
{
    d->m_fileIOThread.quit();
    saveScannerOptions();
    d->m_fileIOThread.wait();
}

QString Skanpage::deviceVendor() const
{
    return d->m_deviceVendor;
}

QString Skanpage::deviceModel() const
{
    return d->m_deviceModel;
}

QString Skanpage::deviceName() const
{
    return d->m_deviceName;
}

void Skanpage::startScan()
{
    d->m_ksaneInterface.startScan();
    d->m_scanInProgress = true;
    d->m_state = ApplicationState::ScanInProgress;
    Q_EMIT applicationStateChanged(d->m_state);
}

Skanpage::ApplicationState Skanpage::applicationState() const
{
    return d->m_state;
}

void Skanpage::imageReady(const QImage &image)
{
    d->m_documentHandler.addImage(image);
    d->m_scannedImages++;
}

void Skanpage::saveScannerOptions()
{
    KConfigGroup options(KSharedConfig::openStateConfig(), QString::fromLatin1("Options For %1").arg(d->m_ksaneInterface.deviceName()));

    QMap<QString, QString> optionMap = d->m_ksaneInterface.getOptionsMap();

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Saving scanner options") << optionMap;
    QMap<QString, QString>::const_iterator it = optionMap.constBegin();
    while (it != optionMap.constEnd()) {
        options.writeEntry(it.key(), it.value());
        ++it;
    }
    options.sync();
}

void Skanpage::loadScannerOptions()
{
    KConfigGroup scannerOptions(KSharedConfig::openStateConfig(), QString::fromLatin1("Options For %1").arg(d->m_ksaneInterface.deviceName()));

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Loading scanner options") << scannerOptions.entryMap();

    d->m_ksaneInterface.setOptionsMap(scannerOptions.entryMap());
}

void Skanpage::availableDevices(const QList<KSaneCore::DeviceInfo> &deviceList)
{
    if (d->m_state == SearchingForDevices) {
        d->m_availableDevices.updateDevicesList(deviceList);

        d->m_state = DeviceSelection;
        Q_EMIT applicationStateChanged(d->m_state);

        // if there is only one scanning device available, open it
        if (d->m_availableDevices.rowCount() == 1) {
            d->m_availableDevices.selectDevice(0);
            qCDebug(SKANPAGE_LOG) << QStringLiteral("Automatically selecting only available device: ") << d->m_availableDevices.getSelectedDeviceName();
            openDevice(d->m_availableDevices.getSelectedDeviceName());
        }
    }
}

bool Skanpage::openDevice(const QString &deviceName, const QString &deviceVendor, const QString &deviceModel)
{
    KSaneCore::KSaneOpenStatus status = KSaneCore::OpeningFailed;
    if (!deviceName.isEmpty()) {
        qCDebug(SKANPAGE_LOG) << QStringLiteral("Trying to open device: %1").arg(deviceName);
        status = d->m_ksaneInterface.openDevice(deviceName);
        if (status == KSaneCore::OpeningSucceeded) {
            if (!deviceVendor.isEmpty()) {
                finishOpeningDevice(deviceName, deviceVendor, deviceModel);
            } else {
                finishOpeningDevice(deviceName, d->m_ksaneInterface.deviceVendor(), d->m_ksaneInterface.deviceModel());
            }
        } else if (status == KSaneCore::OpeningDenied) {
            showUserMessage(SkanpageUtils::ErrorMessage, QStringLiteral("Access to selected device has been denied"));
        } else {
            showUserMessage(SkanpageUtils::ErrorMessage, QStringLiteral("Failed to open selected device."));
        }

    }
    return status == KSaneCore::OpeningSucceeded;
}

void Skanpage::finishOpeningDevice(const QString &deviceName, const QString &deviceVendor, const QString &deviceModel)
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("Finishing opening of device %1 and loading options").arg(deviceName);

    KConfigGroup options(KSharedConfig::openStateConfig(), QStringLiteral("general"));
    options.writeEntry(QStringLiteral("deviceName"), deviceName);
    options.writeEntry(QStringLiteral("deviceModel"), deviceVendor);
    options.writeEntry(QStringLiteral("deviceVendor"), deviceModel);

    d->m_deviceName = deviceName;
    d->m_deviceVendor = deviceVendor;
    d->m_deviceModel = deviceModel;
    Q_EMIT deviceInfoUpdated();
    
    d->m_optionsModel.setOptionsList(d->m_ksaneInterface.getOptionsList());
    Q_EMIT optionsChanged();

    // load saved options
    loadScannerOptions();

    d->m_state = ReadyForScan;
    Q_EMIT applicationStateChanged(d->m_state);
}

void Skanpage::reloadDevicesList()
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("(Re-)loading devices list");

    if (d->m_ksaneInterface.closeDevice()) {
        d->m_deviceName.clear();
        d->m_deviceVendor.clear();
        d->m_deviceModel.clear();
        Q_EMIT deviceInfoUpdated();
        d->m_optionsModel.clearOptions();
        Q_EMIT optionsChanged();
    }
    d->m_state = SearchingForDevices;
    Q_EMIT applicationStateChanged(d->m_state);
    d->m_ksaneInterface.reloadDevicesList();
}

void Skanpage::showKSaneMessage(KSaneIface::KSaneCore::KSaneScanStatus status, const QString &strStatus)
{
    switch (status) {
        case KSaneIface::KSaneCore::ErrorGeneral:
            showUserMessage(SkanpageUtils::ErrorMessage, strStatus);
            break;
        case KSaneIface::KSaneCore::Information:
            showUserMessage(SkanpageUtils::InformationMessage, strStatus);
            break;
        default:
            break;
    }
}

void Skanpage::showUserMessage(SkanpageUtils::MessageLevel level, const QString &text)
{
    Q_EMIT newUserMessage(QVariant(level), QVariant(text));
}

void Skanpage::progressUpdated(int progress)
{
    d->m_progress = progress;
    Q_EMIT progressChanged(d->m_progress);
}

void Skanpage::batchModeCountDown(int remainingSeconds)
{
    d->m_remainingSeconds = remainingSeconds;
    Q_EMIT countDownChanged(d->m_remainingSeconds);
}

int Skanpage::progress() const
{
    return d->m_progress;
}

int Skanpage::countDown() const
{
    return d->m_remainingSeconds;
}

KSaneIface::KSaneCore *Skanpage::ksaneInterface() const
{
    return &d->m_ksaneInterface;
}

DocumentModel *Skanpage::documentModel() const
{
    return &d->m_documentHandler;
}

DevicesModel *Skanpage::devicesModel() const
{
    return &d->m_availableDevices;
}

FormatModel *Skanpage::formatModel() const
{
    return &d->m_formatModel;
}

FilteredOptionsModel *Skanpage::optionsModel() const
{
    return &d->m_filteredOptionsModel;
}

void Skanpage::cancelScan()
{
    d->m_ksaneInterface.stopScan();
}

void Skanpage::imageTemporarilySaved()
{
    d->m_scannedImages--;
    checkFinish();
}

void Skanpage::scanningFinished(KSaneCore::KSaneScanStatus status, const QString &strStatus)
{
    //only print debug, errors are already reported by KSaneCore::userMessage
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Finished scanning! Status code:") << status << QStringLiteral("Status message:") << strStatus;
    
    d->m_scanInProgress = false;
    checkFinish();
}
void Skanpage::checkFinish()
{
    if (d->m_scannedImages == 0 && !d->m_scanInProgress) {
        d->m_state = ApplicationState::ReadyForScan;
        Q_EMIT applicationStateChanged(d->m_state);
    }
}
