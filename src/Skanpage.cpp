/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Skanpage.h"


#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KConfigGroup>
#include <KSharedConfig>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "OptionsModel.h"
#include "SingleOption.h"
#include "skanpage_debug.h"

Skanpage::Skanpage(const QString &deviceName, QObject *parent)
    : QObject(parent)
    , m_ksaneInterface(std::make_unique<KSaneIface::KSaneCore>())
    , m_docHandler(std::make_unique<DocumentModel>())
    , m_availableDevices(std::make_unique<DevicesModel>())
    , m_optionsModel(std::make_unique<OptionsModel>())
    , m_resolutionOption(std::make_unique<SingleOption>())
    , m_pageSizeOption(std::make_unique<SingleOption>())
    , m_sourceOption(std::make_unique<SingleOption>())
    , m_scanModeOption(std::make_unique<SingleOption>())
{
    connect(m_ksaneInterface.get(), &KSaneCore::scannedImageReady, this, &Skanpage::imageReady);
    connect(m_ksaneInterface.get(), &KSaneCore::availableDevices, this, &Skanpage::availableDevices);
    connect(m_ksaneInterface.get(), &KSaneCore::userMessage, this, &Skanpage::showKSaneMessage);
    connect(m_ksaneInterface.get(), &KSaneCore::scanProgress, this, &Skanpage::progressUpdated);
    connect(m_ksaneInterface.get(), &KSaneCore::scanFinished, this, &Skanpage::scanningFinished);
    connect(m_ksaneInterface.get(), &KSaneCore::openedDeviceInfoUpdated, this, &Skanpage::deviceInfoUpdated);
    connect(m_docHandler.get(), &DocumentModel::showUserMessage, this, &Skanpage::showUserMessage);
    connect(m_docHandler.get(), &DocumentModel::newPageAdded, this, &Skanpage::imageTemporarilySaved);

    reloadDevicesList();

    // try to open device from command line option first, then remembered device
    if (deviceName.isEmpty() || !openDevice(deviceName)) {

        KConfigGroup options(KSharedConfig::openConfig(), QStringLiteral("general"));
        const QString savedDeviceName = options.readEntry(QStringLiteral("deviceName"));

        openDevice(savedDeviceName);
    }
}

Skanpage::~Skanpage()
{
    saveScannerOptions();
}

QString Skanpage::deviceVendor() const
{
    return m_ksaneInterface->deviceVendor();
}

QString Skanpage::deviceModel() const
{
    return m_ksaneInterface->deviceModel();
}

QString Skanpage::deviceName() const
{
    return m_ksaneInterface->deviceName();
}

void Skanpage::startScan()
{
    m_ksaneInterface->startScan();
    progressUpdated(0);
    m_scanInProgress = true;
    m_state = ApplicationState::ScanInProgress;
    Q_EMIT applicationStateChanged();
}

Skanpage::ApplicationState Skanpage::applicationState() const
{
    return m_state;
}

void Skanpage::imageReady(const QImage &image)
{
    m_docHandler->addImage(image);
    m_scannedImages++;
}

void Skanpage::showAboutDialog(void)
{
    KAboutApplicationDialog(KAboutData::applicationData()).exec();
}

void Skanpage::saveScannerOptions()
{
    KConfigGroup options(KSharedConfig::openConfig(), QString::fromLatin1("Options For %1").arg(m_ksaneInterface->deviceName()));

    QMap<QString, QString> optionMap = m_ksaneInterface->getOptionsMap();

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
    if (!m_ksaneInterface) {
        return;
    }

    KConfigGroup scannerOptions(KSharedConfig::openConfig(), QString::fromLatin1("Options For %1").arg(m_ksaneInterface->deviceName()));

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Loading scanner options") << scannerOptions.entryMap();

    m_ksaneInterface->setOptionsMap(scannerOptions.entryMap());
}

void Skanpage::availableDevices(const QList<KSaneCore::DeviceInfo> &deviceList)
{
    if (m_state == SearchingForDevices) {
        m_availableDevices->updateDevicesList(deviceList);

        m_state = DeviceSelection;
        Q_EMIT applicationStateChanged();

        // if there is only one scanning device available, open it
        if (m_availableDevices->rowCount() == 1) {
            m_availableDevices->selectDevice(0);
            qCDebug(SKANPAGE_LOG) << QStringLiteral("Automatically selecting only available device: ") << m_availableDevices->getSelectedDeviceName();
            openDevice(m_availableDevices->getSelectedDeviceName());
        }
    }
}

bool Skanpage::openDevice(const QString &deviceName)
{
    KSaneCore::KSaneOpenStatus status = KSaneCore::OpeningFailed;
    if (!deviceName.isEmpty()) {
        qCDebug(SKANPAGE_LOG) << QStringLiteral("Trying to open device: %1").arg(deviceName);
        status = m_ksaneInterface->openDevice(deviceName);
        if (status == KSaneCore::OpeningSucceeded) {
            finishOpeningDevice(deviceName);
        } else if (status == KSaneCore::OpeningDenied) {
            showUserMessage(SkanpageUtils::ErrorMessage, QStringLiteral("Access to selected device has been denied"));
        } else {
            showUserMessage(SkanpageUtils::ErrorMessage, QStringLiteral("Failed to open selected device."));
        }

    }
    return status == KSaneCore::OpeningSucceeded;
}

void Skanpage::finishOpeningDevice(const QString &deviceName)
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("Finishing opening of device %1 and loading options").arg(deviceName);

    KConfigGroup options(KSharedConfig::openConfig(), QStringLiteral("general"));
    options.writeEntry(QStringLiteral("deviceName"), deviceName);

    m_optionsModel->setOptionsList(m_ksaneInterface->getOptionsList());
    m_resolutionOption->setOption(m_ksaneInterface->getOption(KSaneCore::ResolutionOption));
    m_pageSizeOption->setOption(m_ksaneInterface->getOption(KSaneCore::PageSizeOption));
    m_sourceOption->setOption(m_ksaneInterface->getOption(KSaneCore::SourceOption));
    m_scanModeOption->setOption(m_ksaneInterface->getOption(KSaneCore::ScanModeOption));
    Q_EMIT optionsChanged();

    // load saved options
    loadScannerOptions();

    m_state = ReadyForScan;
    Q_EMIT applicationStateChanged();
}

void Skanpage::reloadDevicesList()
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("(Re-)loading devices list");

    if (m_ksaneInterface->closeDevice()) {
        m_optionsModel->clearOptions();
        m_resolutionOption->clearOption();
        m_pageSizeOption->clearOption();
        m_sourceOption->clearOption();
        m_scanModeOption->clearOption();
        Q_EMIT optionsChanged();
    }
    m_state = SearchingForDevices;
    Q_EMIT applicationStateChanged();
    m_ksaneInterface->reloadDevicesList();
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
    m_progress = progress;
    Q_EMIT progressChanged();
}

int Skanpage::progress() const
{
    return m_progress;
}

KSaneIface::KSaneCore *Skanpage::ksaneInterface() const
{
    return m_ksaneInterface.get();
}

DocumentModel *Skanpage::documentModel() const
{
    return m_docHandler.get();
}

DevicesModel *Skanpage::devicesModel() const
{
    return m_availableDevices.get();
}

OptionsModel *Skanpage::optionsModel() const
{
    return m_optionsModel.get();
}

SingleOption *Skanpage::resolutionOption() const
{
    return m_resolutionOption.get();
}

SingleOption *Skanpage::pageSizeOption() const
{
    return m_pageSizeOption.get();
}

SingleOption *Skanpage::sourceOption() const
{
    return m_sourceOption.get();
}

SingleOption *Skanpage::scanModeOption() const
{
    return m_scanModeOption.get();
}

void Skanpage::cancelScan()
{
    m_ksaneInterface->stopScan();
}

void Skanpage::imageTemporarilySaved()
{
    m_scannedImages--;
    checkFinish();
}

void Skanpage::scanningFinished(KSaneCore::KSaneScanStatus status, const QString &strStatus)
{
    //only print debug, errors are already reported by KSaneCore::userMessage
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Finished scanning! Status code:") << status << QStringLiteral("Status message:") << strStatus;
    
    m_scanInProgress = false;
    checkFinish();
}
void Skanpage::checkFinish()
{
    if (m_scannedImages == 0 && !m_scanInProgress) {
        m_state = ApplicationState::ReadyForScan;
        Q_EMIT applicationStateChanged();
    }
}
