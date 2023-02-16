/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Skanpage.h"

#include <QThread>
#include <QtQml>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KShortcutsDialog>

#include "DevicesModel.h"
#include "DocumentModel.h"
#include "OptionsModel.h"
#include "FormatModel.h"
#include "FilteredOptionsModel.h"
#include "DocumentSaver.h"
#include "DocumentPrinter.h"
#include "OCREngine.h"
#include "skanpage_debug.h"

class SkanpagePrivate {
public:
    KSaneCore::Interface m_ksaneInterface;
    DocumentModel m_documentHandler;
    DevicesModel m_availableDevices;
    OptionsModel m_optionsModel;
    FormatModel m_formatModel;
    FilteredOptionsModel m_filteredOptionsModel;
    DocumentSaver m_documentSaver;
    DocumentPrinter m_documentPrinter;
    OCREngine m_OCREngine;
    QThread m_fileIOThread;
    SkanpageConfiguration *m_configuration;
    KActionCollection *m_actionCollection;
    SkanpageState *m_stateConfiguration;

    int m_progress = 100;
    int m_remainingSeconds = 0;
    int m_scannedImages = 0;
    Skanpage::ApplicationState m_state = Skanpage::SearchingForDevices;
    bool m_scanInProgress = false;
    bool m_scanIsPreview = false;
    QRectF m_maximumScanArea;
    QRectF m_scanArea; // Rectangle from (0, 0) to (1, 1)
    QImage m_previewImage;
    QString m_deviceName;
    QString m_deviceVendor;
    QString m_deviceModel;
};

using namespace KSaneCore;

Skanpage::Skanpage(const QString &deviceName, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<SkanpagePrivate>())
{
    d->m_stateConfiguration = SkanpageState::self();
    d->m_configuration = SkanpageConfiguration::self();
    if (d->m_configuration->defaultFolder().isEmpty()) {
        d->m_configuration->setDefaultFolder(QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    }

    d->m_filteredOptionsModel.setSourceModel(&d->m_optionsModel);

    d->m_actionCollection = new KActionCollection(this);

    connect(&d->m_ksaneInterface, &Interface::scannedImageReady, this, &Skanpage::imageReady);
    connect(&d->m_ksaneInterface, &Interface::availableDevices, this, &Skanpage::availableDevices);
    connect(&d->m_ksaneInterface, &Interface::userMessage, this, &Skanpage::showKSaneMessage);
    connect(&d->m_ksaneInterface, &Interface::scanProgress, this, &Skanpage::progressUpdated);
    connect(&d->m_ksaneInterface, &Interface::scanFinished, this, &Skanpage::scanningFinished);
    connect(&d->m_ksaneInterface, &Interface::batchModeCountDown, this, &Skanpage::batchModeCountDown);
    connect(&d->m_documentHandler, &DocumentModel::newPageAdded, this, &Skanpage::imageTemporarilySaved);
    
    d->m_fileIOThread.start();
    d->m_documentSaver.moveToThread(&d->m_fileIOThread);
    d->m_OCREngine.moveToThread(&d->m_fileIOThread);
    d->m_documentSaver.setOCREngine(&d->m_OCREngine);

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
    d->m_configuration->save();
    d->m_stateConfiguration->save();
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

void Skanpage::setupScanningBounds()
{
    Option *tlx = d->m_ksaneInterface.getOption(Interface::TopLeftXOption);
    Option *tly = d->m_ksaneInterface.getOption(Interface::TopLeftYOption);
    Option *brx = d->m_ksaneInterface.getOption(Interface::BottomRightXOption);
    Option *bry = d->m_ksaneInterface.getOption(Interface::BottomRightYOption);

    if (tlx && tly && brx && bry) {
        QVariant tlxMin = tlx->minimumValue(), tlyMin = tly->minimumValue();
        QVariant brxMax = brx->maximumValue(), bryMax = bry->maximumValue();
        if (tlxMin.isValid() && tlyMin.isValid() && brxMax.isValid() && bryMax.isValid()) {
            d->m_maximumScanArea.setCoords(tlxMin.toReal(), tlyMin.toReal(), brxMax.toReal(), bryMax.toReal());
            d->m_scanArea.setCoords(tlx->value().toReal() / d->m_maximumScanArea.width(),
                                    tly->value().toReal() / d->m_maximumScanArea.height(),
                                    brx->value().toReal() / d->m_maximumScanArea.width(),
                                    bry->value().toReal() / d->m_maximumScanArea.height());

            connect(tlx, &Option::valueChanged, this, [&](const QVariant &value){
                d->m_scanArea.setLeft(value.toReal() / d->m_maximumScanArea.width());
                Q_EMIT scanAreaChanged(d->m_scanArea);
            });
            connect(tly, &Option::valueChanged, this, [&](const QVariant &value){
                d->m_scanArea.setTop(value.toReal() / d->m_maximumScanArea.height());
                Q_EMIT scanAreaChanged(d->m_scanArea);
            });
            connect(brx, &Option::valueChanged, this, [&](const QVariant &value){
                d->m_scanArea.setRight(value.toReal() / d->m_maximumScanArea.width());
                Q_EMIT scanAreaChanged(d->m_scanArea);
            });
            connect(bry, &Option::valueChanged, this, [&](const QVariant &value){
                d->m_scanArea.setBottom(value.toReal() / d->m_maximumScanArea.height());
                Q_EMIT scanAreaChanged(d->m_scanArea);
            });
        }
    }
}

QRectF Skanpage::scanArea() const
{
    return d->m_scanArea;
}

void Skanpage::setScanArea(QRectF area)
{
    if (area == d->m_scanArea) return;
    d->m_ksaneInterface.getOption(Interface::TopLeftXOption)->setValue(area.left() * d->m_maximumScanArea.width());
    d->m_ksaneInterface.getOption(Interface::TopLeftYOption)->setValue(area.top() * d->m_maximumScanArea.height());
    d->m_ksaneInterface.getOption(Interface::BottomRightXOption)->setValue(area.right() * d->m_maximumScanArea.width());
    d->m_ksaneInterface.getOption(Interface::BottomRightYOption)->setValue(area.bottom() * d->m_maximumScanArea.height());
}

QImage Skanpage::previewImage() const
{
    return d->m_previewImage;
}

void Skanpage::preview()
{
    if (Option *opt = d->m_ksaneInterface.getOption(Interface::TopLeftXOption)) opt->storeCurrentData();
    if (Option *opt = d->m_ksaneInterface.getOption(Interface::TopLeftYOption)) opt->storeCurrentData();
    if (Option *opt = d->m_ksaneInterface.getOption(Interface::BottomRightXOption)) opt->storeCurrentData();
    if (Option *opt = d->m_ksaneInterface.getOption(Interface::BottomRightYOption)) opt->storeCurrentData();
    if (d->m_maximumScanArea.isValid()) setScanArea(QRectF(0.0, 0.0, 1.0, 1.0));

    if (Option *opt = d->m_ksaneInterface.getOption(Interface::ResolutionOption)) {
        opt->storeCurrentData();
        if (QVariant minRes = opt->minimumValue(); minRes.isValid()) {
            if (opt->type() == Option::TypeValueList) opt->setValue(minRes);
            else opt->setValue(minRes.toInt() < 25 ? 25 : minRes);
        }
    }
    if (Option* opt = d->m_ksaneInterface.getOption(Interface::PreviewOption)) opt->setValue(true);

    d->m_scanIsPreview = true;

    startScan();
}

void Skanpage::finishPreview()
{
    if (Option* opt = d->m_ksaneInterface.getOption(Interface::TopLeftXOption)) opt->restoreSavedData();
    if (Option* opt = d->m_ksaneInterface.getOption(Interface::TopLeftYOption)) opt->restoreSavedData();
    if (Option* opt = d->m_ksaneInterface.getOption(Interface::BottomRightXOption)) opt->restoreSavedData();
    if (Option* opt = d->m_ksaneInterface.getOption(Interface::BottomRightYOption)) opt->restoreSavedData();

    if (Option* opt = d->m_ksaneInterface.getOption(Interface::ResolutionOption)) opt->restoreSavedData();
    if (Option* opt = d->m_ksaneInterface.getOption(Interface::PreviewOption)) opt->setValue(false);

    d->m_scanIsPreview = false;
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
    if (d->m_scanIsPreview) {
        d->m_ksaneInterface.stopScan(); // Needed for ADF
        finishPreview();
        d->m_previewImage = image;
        Q_EMIT previewImageChanged(d->m_previewImage);
    } else {
        d->m_documentHandler.addImage(image);
        d->m_scannedImages++;
    }
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

void Skanpage::availableDevices(const QList<DeviceInformation *> &deviceList)
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
    Interface::OpenStatus status = Interface::OpeningFailed;
    if (!deviceName.isEmpty()) {
        qCDebug(SKANPAGE_LOG) << QStringLiteral("Trying to open device: %1").arg(deviceName);
        status = d->m_ksaneInterface.openDevice(deviceName);
        if (status == Interface::OpeningSucceeded) {
            if (!deviceVendor.isEmpty()) {
                finishOpeningDevice(deviceName, deviceVendor, deviceModel);
            } else {
                finishOpeningDevice(deviceName, d->m_ksaneInterface.deviceVendor(), d->m_ksaneInterface.deviceModel());
            }
        } else if (status == Interface::OpeningDenied) {
            showUserMessage(SkanpageUtils::ErrorMessage, QStringLiteral("Access to selected device has been denied"));
        } else {
            showUserMessage(SkanpageUtils::ErrorMessage, QStringLiteral("Failed to open selected device."));
        }

    }
    return status == Interface::OpeningSucceeded;
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

    setupScanningBounds();

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
    d->m_ksaneInterface.reloadDevicesList(d->m_configuration->showAllDevices() ? Interface::DeviceType::AllDevices : Interface::DeviceType::NoCameraAndVirtualDevices);
}

void Skanpage::showKSaneMessage(Interface::ScanStatus status, const QString &strStatus)
{
    switch (status) {
        case Interface::ErrorGeneral:
            showUserMessage(SkanpageUtils::ErrorMessage, strStatus);
            break;
        case Interface::Information:
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

Interface *Skanpage::ksaneInterface() const
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

OCRLanguageModel *Skanpage::languageModel() const
{
    return d->m_OCREngine.languages();
}

SkanpageConfiguration *Skanpage::configuration() const
{
    return d->m_configuration;
}

SkanpageState *Skanpage::stateConfiguration() const
{
    return d->m_stateConfiguration;
}

bool Skanpage::OCRavailable() const
{
    return d->m_OCREngine.available();
}

void Skanpage::print() {
    d->m_documentPrinter.printDocument(d->m_documentHandler.selectPages(QList<int>()));
}

void Skanpage::registerAction(QObject* item, QObject* shortcuts, const QString &iconText)
{
    auto getQKeySequence = [](const QVariant &variant) -> QKeySequence {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (variant.type() == QVariant::KeySequence) return variant.value<QKeySequence>();
        else if (variant.type() == QVariant::String) return variant.value<QString>();
#else
        if (variant.typeId() == QMetaType::QKeySequence) return variant.value<QKeySequence>();
        else if (variant.typeId() == QMetaType::QString) return variant.value<QString>();
#endif
        else return variant.value<QKeySequence::StandardKey>();
    };

    auto getKStandardShortcuts = [](const QVariant &variant) -> QList<QKeySequence> {
        auto id = KStandardShortcut::findByName(variant.toString());
        if (id != KStandardShortcut::AccelNone) {
            return KStandardShortcut::shortcut(id);
        } else {
            qCDebug(SKANPAGE_LOG) << "Invalid KStandardShortcut specified from QML" << variant.toString();
            return QList<QKeySequence>();
        }
    };

    QString id = QQmlEngine::contextForObject(item)->nameForObject(item);

    QAction *act = d->m_actionCollection->addAction(id);
    act->setText(item->property("text").toString());
    act->setIcon(QIcon::fromTheme(iconText));
    act->setIconVisibleInMenu(true);

    QList<QKeySequence> sequences;
    if (QVariant prop = item->property("shortcut"); prop.isValid()) {
        QKeySequence seq = getQKeySequence(prop);
        if (!seq.isEmpty()) sequences.append(seq);
    }
    if (QVariant prop = item->property("shortcutsName"); prop.isValid() && !prop.toString().isEmpty()) {
        sequences.append(getKStandardShortcuts(prop));
    }
    d->m_actionCollection->setDefaultShortcuts(act, sequences);
    d->m_actionCollection->readSettings();

    auto updateKeySequences = [=]() {
        if (act->shortcuts().isEmpty()) return;
        item->setProperty("shortcut", act->shortcuts().front());
        QList<QVariant> sequenceList;
        for (int i = 1; i < act->shortcuts().size(); i++) sequenceList.append(act->shortcuts().at(i));
        shortcuts->setProperty("sequences", sequenceList);
    };
    updateKeySequences(); // Move the specified shortcut to the QML Shortcut object
    connect(act, &QAction::changed, this, updateKeySequences);
}

void Skanpage::showShortcutsDialog() {
    KShortcutsDialog::showDialog(d->m_actionCollection);
}

void Skanpage::cancelScan()
{
    if (d->m_progress > 0 && d->m_ksaneInterface.scanImage()) {
        d->m_ksaneInterface.lockScanImage();
        QImage image = *d->m_ksaneInterface.scanImage();
        d->m_ksaneInterface.unlockScanImage();
        imageReady(image);
    }

    d->m_ksaneInterface.stopScan();
}

void Skanpage::imageTemporarilySaved()
{
    d->m_scannedImages--;
    checkFinish();
}

void Skanpage::scanningFinished(Interface::ScanStatus status, const QString &strStatus)
{
    //only print debug, errors are already reported by Interface::userMessage
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
