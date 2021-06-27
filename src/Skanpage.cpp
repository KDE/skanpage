/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Skanpage.h"

#include <QPageSize>
#include <QStringList>

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "OptionsModel.h"
#include "SingleOption.h"
#include "skanpage_debug.h"

Skanpage::Skanpage(const QString &deviceName, QObject *parent)
    : QObject(parent)
    , m_ksanew(std::make_unique<KSaneIface::KSaneWidget>())
    , m_docHandler(std::make_unique<DocumentModel>())
    , m_availableDevices(std::make_unique<DevicesModel>())
    , m_optionsModel(std::make_unique<OptionsModel>())
    , m_resolutionOption(std::make_unique<SingleOption>())
    , m_pageSizeOption(std::make_unique<SingleOption>())
    , m_sourceOption(std::make_unique<SingleOption>())
    , m_scanModeOption(std::make_unique<SingleOption>())
{
    connect(m_ksanew.get(), &KSaneWidget::scannedImageReady, this, &Skanpage::imageReady);
    connect(m_ksanew.get(), &KSaneWidget::availableDevices, this, &Skanpage::availableDevices);
    connect(m_ksanew.get(), &KSaneWidget::userMessage, this, &Skanpage::showKSaneMessage);
    connect(m_ksanew.get(), &KSaneWidget::scanProgress, this, &Skanpage::progressUpdated);
    connect(m_ksanew.get(), &KSaneWidget::scanDone, this, &Skanpage::scanDone);
    connect(m_ksanew.get(), &KSaneWidget::openedDeviceInfoUpdated, this, &Skanpage::deviceInfoUpdated);
    connect(m_docHandler.get(), &DocumentModel::showUserMessage, this, &Skanpage::showUserMessage);

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
    return m_ksanew->deviceVendor();
}

QString Skanpage::deviceModel() const
{
    return m_ksanew->deviceModel();
}

QString Skanpage::deviceName() const
{
    return m_ksanew->deviceName();
}

void Skanpage::startScan()
{
    m_scanInProgress = true;
    Q_EMIT scanInProgressChanged(true);
    m_ksanew->scanFinal();
}

Skanpage::ApplicationState Skanpage::applicationState() const
{
    return m_state;
}

bool Skanpage::scanInProgress() const
{
    return m_scanInProgress;
}

void Skanpage::imageReady(const QImage &image)
{   
    m_docHandler->addImage(image, m_resolutionOption->value().toInt());
}

void Skanpage::showAboutDialog(void)
{
    KAboutApplicationDialog(KAboutData::applicationData()).exec();
}

void Skanpage::saveScannerOptions()
{
    KConfigGroup saving(KSharedConfig::openConfig(), "Image Saving");

    if (!m_ksanew) {
        return;
    }
    
    KConfigGroup options(KSharedConfig::openConfig(), QString::fromLatin1("Options For %1").arg(m_ksanew->deviceName()));

    QMap<QString, QString> opts;
    m_ksanew->getOptVals(opts);
    
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Saving scanner options") << opts;
    QMap<QString, QString>::const_iterator it = opts.constBegin();
    while (it != opts.constEnd()) {
        options.writeEntry(it.key(), it.value());
        ++it;
    }
    options.sync();
}

void Skanpage::loadScannerOptions()
{
    KConfigGroup saving(KSharedConfig::openConfig(), "Image Saving");

    if (!m_ksanew) {
        return;
    }

    KConfigGroup scannerOptions(KSharedConfig::openConfig(), QString::fromLatin1("Options For %1").arg(m_ksanew->deviceName()));

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Loading scanner options") << scannerOptions.entryMap();

    m_ksanew->setOptVals(scannerOptions.entryMap());
}

void Skanpage::availableDevices(const QList<KSaneWidget::DeviceInfo> &deviceList)
{
    if (m_state != ReadyForScan) {
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
    bool success = false;
    if (!deviceName.isEmpty()) {
        qCDebug(SKANPAGE_LOG) << QStringLiteral("Trying to open device: %1").arg(deviceName);
        success = m_ksanew->openDevice(deviceName);
        if (success) {
            finishOpeningDevice(deviceName);
        }
    }
    return success;
}

void Skanpage::finishOpeningDevice(const QString &deviceName)
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("Finishing opening of device %1 and loading options").arg(deviceName);
    
    KConfigGroup options(KSharedConfig::openConfig(), QStringLiteral("general"));
    options.writeEntry(QStringLiteral("deviceName"), deviceName);

    m_ksanew->enableAutoSelect(false);

    m_optionsModel->setOptionsList(m_ksanew->getOptionsList());
    m_resolutionOption->setOption(m_ksanew->getOption(KSaneWidget::ResolutionOption));
    m_pageSizeOption->setOption(m_ksanew->getOption(KSaneWidget::PageSizeOption));
    m_sourceOption->setOption(m_ksanew->getOption(KSaneWidget::SourceOption));
    m_scanModeOption->setOption(m_ksanew->getOption(KSaneWidget::ScanModeOption));
    Q_EMIT optionsChanged();

    // load saved options
    loadScannerOptions();

    m_state = ReadyForScan;
    Q_EMIT applicationStateChanged();
}

void Skanpage::reloadDevicesList()
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("(Re-)loading devices list");
    
    if (m_ksanew->closeDevice()) {
        m_optionsModel->clearOptions();
        m_resolutionOption->clearOption();
        m_pageSizeOption->clearOption();
        m_sourceOption->clearOption();
        m_scanModeOption->clearOption();
        Q_EMIT optionsChanged();
        m_state = SearchingForDevices;
        Q_EMIT applicationStateChanged();
        m_ksanew->initGetDeviceList();
    }
}

void Skanpage::showKSaneMessage(int status, const QString &strStatus)
{
    switch (status) {
        case KSaneWidget::ErrorGeneral:
            showUserMessage(Skanpage::ErrorMessage, strStatus);
            break;
        case KSaneWidget::ErrorCannotSegment:
            showUserMessage(Skanpage::ErrorMessage, strStatus);
            break;
        case KSaneWidget::Information:
            showUserMessage(Skanpage::InformationMessage, strStatus);
            break;
        default:
            break;
    }
}

void Skanpage::showUserMessage(Skanpage::MessageLevel level, const QString &text)
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
    m_ksanew->scanCancel();
}

void Skanpage::scanDone(int status, const QString &strStatus)
{
    //only print debug, errors are already reported by KSaneWidget::userMessage
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Finished scanning! Status code:") << status << QStringLiteral("Status message:") << strStatus;
    m_progress = 100;
    Q_EMIT progressChanged();
    m_scanInProgress = false;
    Q_EMIT scanInProgressChanged(false);
}
