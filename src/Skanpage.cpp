/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Skanpage.h"

#include <QDebug>
#include <QPageSize>
#include <QPrinter>
#include <QStringList>
#include <QTemporaryFile>

#include <KAboutApplicationDialog>
#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "skanpage_debug.h"
#include <errno.h>

Skanpage::Skanpage(const QString &deviceName, QObject *parent)
    : QObject(parent)
    , m_ksanew(std::make_unique<KSaneIface::KSaneWidget>(nullptr))
    , m_docHandler(std::make_unique<DocumentModel>(nullptr))
    , m_availableDevices(std::make_unique<DevicesModel>(nullptr))
    , m_scanSizeIndex(-1)
    , m_progress(100)
    , m_openedDevice(false)
    , m_searchingForDevices(false)
{
    connect(m_ksanew.get(), &KSaneWidget::imageReady, this, &Skanpage::imageReady);
    connect(m_ksanew.get(), &KSaneWidget::availableDevices, this, &Skanpage::availableDevices);
    connect(m_ksanew.get(), &KSaneWidget::userMessage, this, &Skanpage::alertUser);
    connect(m_ksanew.get(), &KSaneWidget::scanProgress, this, &Skanpage::progressUpdated);
    connect(m_ksanew.get(), &KSaneWidget::scanDone, this, &Skanpage::scanDone);
    connect(m_ksanew.get(), &KSaneWidget::openedDeviceInfoUpdated, this, &Skanpage::deviceInfoUpdated);
    
    reloadDevicesList();

    m_scanSizesText << i18n("A4");
    m_scanSizesText << i18n("B5");
    m_scanSizesText << i18n("Letter");
    m_scanSizesText << i18n("Legal");
    m_scanSizesText << i18n("Executive");
    m_scanSizesText << i18n("A5");
    m_scanSizesText << i18n("A6");
    m_scanSizesText << i18n("Custom");

    m_scanSizesEnum << QPageSize::A4;
    m_scanSizesEnum << QPageSize::B5;
    m_scanSizesEnum << QPageSize::Letter;
    m_scanSizesEnum << QPageSize::Legal;
    m_scanSizesEnum << QPageSize::Executive;
    m_scanSizesEnum << QPageSize::A5;
    m_scanSizesEnum << QPageSize::A6;
    m_scanSizesEnum << QPageSize::Custom;

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
    m_ksanew->scanFinal();
}

void Skanpage::showScannerUI()
{
    m_ksanew->show();
}

float Skanpage::scanDPI() const
{
    return m_ksanew->currentDPI();
}

void Skanpage::setScanDPI(float dpi)
{
    if (m_openedDevice) {
        m_ksanew->setOptVal(QStringLiteral("resolution"), QString::number(dpi));
    }
}

bool Skanpage::colorMode() const
{
    if (m_openedDevice) {
        QString readValue;
        m_ksanew->getOptVal(QStringLiteral("mode"), readValue);
        return QString::compare(readValue, QStringLiteral("Color")) == 0;
    } else {
        return true;
    }
}

void Skanpage::setColorMode(bool colorMode)
{
    if (m_openedDevice) {
        if (colorMode) {
            m_ksanew->setOptVal(QStringLiteral("mode"), QStringLiteral("Color"));
        } else {
            m_ksanew->setOptVal(QStringLiteral("mode"), QStringLiteral("Gray"));
        }
    }
}

int Skanpage::scanSizeIndex() const
{
    return m_scanSizeIndex;
}

void Skanpage::setScanSizeIndex(int index)
{
    if (m_scanSizeIndex != index && m_openedDevice) {
        if (index < 0 || index >= m_scanSizesEnum.size()) {
            index = 0;
        }
        QPageSize ps(m_scanSizesEnum[index]);
        QRectF rectf = ps.rect(QPageSize::Millimeter);
        qCDebug(SKANPAGE_LOG) << QStringLiteral("Selected scan size:") << rectf.topLeft() << rectf.bottomRight();
        qCDebug(SKANPAGE_LOG) << QStringLiteral("Maximum reported sizes are:") << m_ksanew->scanAreaWidth() << m_ksanew->scanAreaHeight();
        if (m_ksanew->scanAreaWidth() < rectf.width()) {
            rectf.setWidth(m_ksanew->scanAreaWidth());
            // set index to custom only when rounded values do not match
            if (qRound(m_ksanew->scanAreaWidth()) < rectf.width()) {
                index = m_scanSizesEnum.size() - 1;
            }
        }
        if (m_ksanew->scanAreaHeight() < rectf.height()) {
            rectf.setHeight(m_ksanew->scanAreaHeight());
            if (qRound(m_ksanew->scanAreaHeight()) < rectf.height()) {
                index = m_scanSizesEnum.size() - 1;
            }
        }
        QString tlx = QString::number(rectf.topLeft().x(), 'f', 1);
        QString tly = QString::number(rectf.topLeft().y(), 'f', 1);
        QString brx = QString::number(rectf.bottomRight().x(), 'f', 1);
        QString bry = QString::number(rectf.bottomRight().y(), 'f', 1);
        m_ksanew->setOptVal(QStringLiteral("tl-x"), tlx);
        m_ksanew->setOptVal(QStringLiteral("tl-y"), tly);
        m_ksanew->setOptVal(QStringLiteral("br-x"), brx);
        m_ksanew->setOptVal(QStringLiteral("br-y"), bry);

        m_scanSizeIndex = index;
        Q_EMIT scanSizeChanged();
    }
}

int Skanpage::pageSizeToIndex(int id)
{
    for (int i = 0; i < m_scanSizesEnum.size(); ++i) {
        if (id == m_scanSizesEnum[i]) {
            return i;
        }
    }
    return m_scanSizesEnum.last(); // custom
}

QString Skanpage::errorMessage() const
{
    return m_errorMessage;
}

const QStringList Skanpage::scanSizes() const
{
    return m_scanSizesText;
}

bool Skanpage::openedDevice() const
{
    return m_openedDevice;
}

bool Skanpage::searchingForDevices() const
{
    return m_searchingForDevices;
}

void Skanpage::imageReady(QByteArray &data, int width, int height, int bytesPerLine, int format)
{
    if ((format == KSaneIface::KSaneWidget::FormatRGB_16_C) || (format == KSaneIface::KSaneWidget::FormatGrayScale16)) {
        signalErrorMessage(i18n("We do not support 16 per color scans at the moment!"));
        return;
    }

    const QImage image = m_ksanew->toQImage(data, width, height, bytesPerLine, static_cast<KSaneIface::KSaneWidget::ImageFormat>(format));

    QTemporaryFile *tmp = new QTemporaryFile(m_docHandler.get());
    tmp->open();
    if (image.save(tmp, "PNG")) {
        if (m_scanSizesEnum[m_scanSizeIndex] == QPageSize::Custom) {
            const float conversionFactorMM = scanDPI() / 25.4;
            m_docHandler->addImage(tmp, QPageSize(QSizeF(width / conversionFactorMM, height/ conversionFactorMM), QPageSize::Millimeter), scanDPI());
        } else {
            m_docHandler->addImage(tmp, QPageSize(m_scanSizesEnum[m_scanSizeIndex]), scanDPI());
        }
    } else {
        signalErrorMessage(i18n("Failed to save image"));
    }
    tmp->close();
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
    QMap<QString, QString>::const_iterator it = opts.constBegin();
    while (it != opts.constEnd()) {
        options.writeEntry(it.key(), it.value());
        ++it;
    }
    options.sync();
}

void Skanpage::defaultScannerOptions()
{
    if (!m_ksanew) {
        return;
    }

    m_ksanew->setOptVals(m_defaultScanOpts);
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
    m_availableDevices->updateDevicesList(deviceList);

    m_searchingForDevices = false;
    Q_EMIT searchingForDevicesChanged();
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

    // save the default sane options for later use
    m_ksanew->getOptVals(m_defaultScanOpts); // FIXME -> m_def... = m_ksanew->optVals();
    m_ksanew->setScanButtonHidden(true);

    // try to set default resolution to 150
    m_ksanew->setOptVal(QStringLiteral("resolution"), QStringLiteral("150"));

    m_ksanew->enableAutoSelect(false);

    // load saved options
    loadScannerOptions();

    m_openedDevice = true;

    setScanSizeIndex(pageSizeToIndex(QPageSize::A4));
    Q_EMIT openedDeviceChanged();
    Q_EMIT scanDPIChanged();
    Q_EMIT scanSizesChanged();
    Q_EMIT colorModeChanged();
}

void Skanpage::reloadDevicesList()
{
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("(Re-)loading devices list");
    if (m_ksanew->closeDevice()) {
        m_openedDevice = false;
        Q_EMIT openedDeviceChanged();
        m_searchingForDevices = true;
        Q_EMIT searchingForDevicesChanged();
        m_ksanew->initGetDeviceList();
    }
}

void Skanpage::alertUser(int status, const QString &strStatus)
{
    Q_UNUSED(status);
    signalErrorMessage(strStatus);
}

void Skanpage::signalErrorMessage(const QString &text)
{
    m_errorMessage = text;
    Q_EMIT errorMessageChanged();
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
}
