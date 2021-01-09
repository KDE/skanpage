/* ============================================================
*
* Copyright (C) 2015 by Kåre Särs <kare.sars@iki .fi>
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

#include "Skanpage.h"

#include <QStringList>
#include <QTemporaryFile>
#include <QDebug>
#include <QPageSize>
#include <QPrinter>
#include <QSettings>

#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>

#include <errno.h>
#include "skanpage_debug.h"
#include "DocumentModel.h"
#include "DevicesModel.h"

Skanpage::Skanpage(const QString &deviceName, QObject *parent)
: QObject(parent)
, m_ksanew(std::make_unique<KSaneIface::KSaneWidget>(nullptr))
, m_docHandler(std::make_unique<DocumentModel>(nullptr))
, m_availableDevices(std::make_unique<DevicesModel>(nullptr))
, m_deviceName(deviceName)
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
}

Skanpage::~Skanpage()
{
    saveScannerOptions();
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
            //set index to custom only when rounded values do not match
            if (qRound(m_ksanew->scanAreaWidth()) < rectf.width()) {
                 index = m_scanSizesEnum.size()-1;
            }
        }
        if (m_ksanew->scanAreaHeight() < rectf.height()) {
            rectf.setHeight(m_ksanew->scanAreaHeight());
            if (qRound(m_ksanew->scanAreaHeight()) < rectf.height()) {
                 index = m_scanSizesEnum.size()-1;
            }
        }
        QString tlx = QString::number(rectf.topLeft().x(),'f',1);
        QString tly = QString::number(rectf.topLeft().y(),'f',1);
        QString brx = QString::number(rectf.bottomRight().x(),'f',1);
        QString bry = QString::number(rectf.bottomRight().y(),'f',1);
        m_ksanew->setOptVal(QStringLiteral("tl-x"), tlx);
        m_ksanew->setOptVal(QStringLiteral("tl-y"), tly);
        m_ksanew->setOptVal(QStringLiteral("br-x"), brx);
        m_ksanew->setOptVal(QStringLiteral("br-y"), bry);
        
        m_scanSizeIndex = index;
        emit scanSizeChanged();
    }
}

int Skanpage::pageSizeToIndex(int id)
{
    for (int i=0; i < m_scanSizesEnum.size(); ++i) {
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

void Skanpage::imageReady(QByteArray &data, int w, int h, int bpl, int f)
{
    // save the image data
    m_data = data;
    m_width = w;
    m_height = h;
    m_bytesPerLine = bpl;
    m_format = f;

    // Save
    if ((m_format == KSaneIface::KSaneWidget::FormatRGB_16_C) ||
        (m_format == KSaneIface::KSaneWidget::FormatGrayScale16))
    {
        signalErrorMessage(i18n("We do not support 16 per color scans at the moment!"));
        return;
    }

    m_img = m_ksanew->toQImage(m_data, m_width, m_height, m_bytesPerLine, (KSaneIface::KSaneWidget::ImageFormat)m_format);

    QTemporaryFile *tmp = new QTemporaryFile(m_docHandler.get());
    tmp->open();
    if (m_img.save(tmp, "PNG")) {
    
        m_docHandler->addImage(tmp, m_scanSizesEnum[m_scanSizeIndex], scanDPI());
    }
    else {
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

    KConfigGroup options(KSharedConfig::openConfig(), QString::fromLatin1("Options For %1").arg(m_deviceName));
    QMap <QString, QString> opts;
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

    KConfigGroup scannerOptions(KSharedConfig::openConfig(), QString::fromLatin1("Options For %1").arg(m_deviceName));
    
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Loading scanner options") << scannerOptions.entryMap();
    
    m_ksanew->setOptVals(scannerOptions.entryMap());
}

void Skanpage::availableDevices(const QList<KSaneWidget::DeviceInfo> &deviceList)
{
    m_availableDevices->updateDevicesList(deviceList);
    QSettings settings(KAboutData::applicationData().organizationDomain(), KAboutData::applicationData().componentName());
    const auto savedDeviceName = settings.value(QStringLiteral("deviceName")).toString();
    
    //try command line option first, then remembered device
    if (!openDevice(m_deviceName)) {
        openDevice(savedDeviceName);
    } 
    m_searchingForDevices = false;
    searchingForDevicesChanged();
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
    QSettings settings(KAboutData::applicationData().organizationDomain(), KAboutData::applicationData().componentName());
    settings.setValue(QStringLiteral("deviceName"), deviceName);
    
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
    openedDeviceChanged();
    scanDPIChanged();
    scanSizesChanged();
    colorModeChanged();
}

void Skanpage::reloadDevicesList() {
    
    qCDebug(SKANPAGE_LOG()) << QStringLiteral("(Re-)loading devices list");
    if (m_ksanew->closeDevice()) {
        m_openedDevice = false;
        openedDeviceChanged();
        m_deviceName.clear();
        m_searchingForDevices = true;
        searchingForDevicesChanged();
        m_ksanew->initGetDeviceList();
    }
}

void Skanpage::alertUser(int type, const QString &strStatus)
{
    Q_UNUSED(type)
    signalErrorMessage(strStatus);
}

void Skanpage::signalErrorMessage(const QString &text)
{
    m_errorMessage = text;
    errorMessageChanged();
}

void Skanpage::progressUpdated(int progress)
{
    m_progress = progress;
    emit progressChanged();
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
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Finished scanning! Status code:") << status << QStringLiteral("Status message:") << strStatus;
    m_progress = 100;
    emit progressChanged();
}
