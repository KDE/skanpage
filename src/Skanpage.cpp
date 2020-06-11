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

#include <QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QPageSize>
#include <QPrinter>

#include <KAboutApplicationDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <KIO/StatJob>
#include <kio/global.h>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KHelpClient>

#include <errno.h>

#include "DocumentModel.h"

Skanpage::Skanpage(const QString &device, QWidget *parent)
: QDialog(parent)
, m_aboutData(nullptr)
, m_ksanew(new KSaneIface::KSaneWidget(nullptr))
, m_docHandler(nullptr)
, m_progress(100)
{
    connect(m_ksanew, &KSaneWidget::imageReady, this, &Skanpage::imageReady);
    connect(m_ksanew, &KSaneWidget::availableDevices, this, &Skanpage::availableDevices);
    connect(m_ksanew, &KSaneWidget::userMessage, this, &Skanpage::alertUser);
    connect(m_ksanew, &KSaneWidget::buttonPressed, this, &Skanpage::buttonPressed);
    connect(m_ksanew, &KSaneWidget::scanProgress, this, &Skanpage::progressUpdated);
    connect(m_ksanew, &KSaneWidget::scanDone, this, &Skanpage::scanDone);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    mainLayout->addWidget(m_ksanew);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);


    m_ksanew->initGetDeviceList();

    // open the scan device
    if (m_ksanew->openDevice(device) == false) {
        QString dev = m_ksanew->selectDevice(nullptr);
        if (dev.isEmpty()) {
            // either no scanner was found or then cancel was pressed.
            exit(0);
        }
        if (m_ksanew->openDevice(dev) == false) {
            // could not open a scanner
            KMessageBox::sorry(nullptr, i18n("Opening the selected scanner failed."));
            exit(1);
        }
        else {
            m_deviceName = QString::fromLatin1("%1:%2").arg(m_ksanew->make()).arg(m_ksanew->model());
        }
    }
    else {
        m_deviceName = device;
    }

    // save the default sane options for later use
    m_ksanew->getOptVals(m_defaultScanOpts); // FIXME -> m_def... = m_ksanew->optVals();
    m_ksanew->setScanButtonHidden(true);

    // try to set default resolution to 150
    m_ksanew->setOptVal(QStringLiteral("resolution"), QStringLiteral("150"));


    m_ksanew->enableAutoSelect(false);

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

    m_scanSizesF << QPageSize::definitionSize(QPageSize::A4);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::B5);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::Letter);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::Legal);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::Executive);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::A5);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::A6);
    m_scanSizesF << QPageSize::definitionSize(QPageSize::Custom);

    QPrinter pd;
    int tmp = pd.pageSize();

    setScanSizeIndex(pageSizeToIndex(tmp));

    // load saved options
    loadScannerOptions();
}

Skanpage::~Skanpage()
{
    saveScannerOptions();
    delete m_ksanew;
}


void Skanpage::startScan()
{
    m_ksanew->scanFinal();
}


void Skanpage::showScannerUI()
{
    show();
}

float Skanpage::scanDPI() const
{
    return m_ksanew->currentDPI();
}

bool Skanpage::setScanDPI(float dpi)
{
    return m_ksanew->setOptVal(QStringLiteral("resolution"), QString::number(dpi));
}

int Skanpage::scanSizeIndex() const
{
    return m_scanSizeIndex;
}

void Skanpage::setScanSizeIndex(int index)
{
    if (m_scanSizeIndex != index) {
        if (index < 0 || index >= m_scanSizesEnum.size()) {
            index = 0;
        }
        QPageSize ps(m_scanSizesEnum[index]);
        QRectF rectf = ps.rect(QPageSize::Millimeter);
        qDebug() << rectf.topLeft() << rectf.bottomRight();
        if (m_ksanew->scanAreaWidth() < rectf.width()) {
            index = m_scanSizesEnum.size()-1;
            rectf.setWidth(m_ksanew->scanAreaWidth());

        }
        if (m_ksanew->scanAreaHeight() < rectf.height()) {
            index = m_scanSizesEnum.size()-1;
            rectf.setWidth(m_ksanew->scanAreaHeight());
        }
        m_ksanew->setSelection(rectf.topLeft(), rectf.bottomRight());
        QString tlx;
        QString tly;
        QString brx;
        QString bry;
        m_ksanew->getOptVal(QStringLiteral("tl-x"), tlx);
        m_ksanew->getOptVal(QStringLiteral("tl-y"), tly);
        m_ksanew->getOptVal(QStringLiteral("br-x"), brx);
        m_ksanew->getOptVal(QStringLiteral("br-y"), bry);
        qDebug() << tlx << tly << brx << bry;
        m_scanSizeIndex = index;
        emit scanSizeChanged();
    }
}

int Skanpage::pageSizeToIndex(int id)
{
    for (int i=0; i<m_scanSizesEnum.size(); ++i) {
        if (id == m_scanSizesEnum[i]) {
            return i;
        }
    }
    return m_scanSizesEnum.last(); // custom
}

const QStringList Skanpage::scanSizes() const
{
    return m_scanSizesText;
}

const QVariantList Skanpage::scanSizesF() const
{
    return m_scanSizesF;
}


const QSize Skanpage::windowSize() const
{
    KConfigGroup window(KSharedConfig::openConfig(), "Window");
    return window.readEntry("Geometry", QSize(740, 400));
}

void Skanpage::showHelp()
{
    KHelpClient::invokeHelp(QLatin1String("index"), QLatin1String("skanlite"));
}

void Skanpage::setAboutData(KAboutData *aboutData)
{
    m_aboutData = aboutData;
}

void Skanpage::saveWindowSize(const QSize &size)
{
    KConfigGroup window(KSharedConfig::openConfig(), "Window");
    window.writeEntry("Geometry", size);
    window.sync();
}


// Pops up message box similar to what perror() would print
//************************************************************
static void perrorMessageBox(const QString &text)
{
    if (errno != 0) {
        KMessageBox::sorry(nullptr, i18n("%1: %2", text, QString::fromLocal8Bit(strerror(errno))));
    } else {
        KMessageBox::sorry(nullptr, text);
    }
}


void Skanpage::setDocument(DocumentModel *handler)
{
    if (m_docHandler) {
        disconnect(m_docHandler, &QObject::destroyed, this, &Skanpage::documentDeleted);
    }
    m_docHandler = handler;
    if (m_docHandler) {
        connect(m_docHandler, &QObject::destroyed, this, &Skanpage::documentDeleted);
    }
}

void Skanpage::documentDeleted()
{
    m_docHandler = nullptr;
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
        perrorMessageBox(i18n("We do not support 16 per color scans at the moment!"));
        return;
    }

    if (!m_docHandler) {
        perrorMessageBox(i18n("No document open for the new page!"));
        return;
    }

    m_img = m_ksanew->toQImage(m_data, m_width, m_height, m_bytesPerLine, (KSaneIface::KSaneWidget::ImageFormat)m_format);

    QTemporaryFile *tmp = new QTemporaryFile(m_docHandler);
    tmp->open();
    if (m_img.save(tmp, "JPEG")) {
        m_docHandler->addImage(tmp);
    }
    else {
        perrorMessageBox(i18n("Failed to save image"));
    }
    tmp->close();
}

void Skanpage::showAboutDialog(void)
{
    KAboutApplicationDialog(*m_aboutData).exec();
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
    m_ksanew->setOptVals(scannerOptions.entryMap());
}

void Skanpage::availableDevices(const QList<KSaneWidget::DeviceInfo> &deviceList)
{
    for (int i = 0; i < deviceList.size(); ++i) {
        qDebug() << deviceList.at(i).name;
    }
}

void Skanpage::alertUser(int type, const QString &strStatus)
{
    switch (type) {
    case KSaneWidget::ErrorGeneral:
        KMessageBox::sorry(nullptr, strStatus, QLatin1String("Skanpage Test"));
        break;
    default:
        KMessageBox::information(nullptr, strStatus, QLatin1String("Skanpage Test"));
    }
}

void Skanpage::buttonPressed(const QString &optionName, const QString &optionLabel, bool pressed)
{
    qDebug() << "Button" << optionName << optionLabel << ((pressed) ? "pressed" : "released");
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

void Skanpage::cancelScan()
{
    m_ksanew->scanCancel();
}

void Skanpage::scanDone(int /*status*/, const QString &/*strStatus*/)
{
    // FIXME status and strStatus
    m_progress = 100;
    emit progressChanged();
}
