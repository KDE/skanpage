/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "Skanpage.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QtQml>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KShortcutsDialog>

#include "DevicesModel.h"
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
    Skanpage::ScanSplit m_scanSplit = Skanpage::ScanNotSplit;
    QList<QRectF> m_scanSubAreas;
    bool m_scanAreaConnectionsDone = false;
    QImage m_previewImage;
    QString m_deviceName;
    QString m_deviceVendor;
    QString m_deviceModel;
    QUrl m_dumpOptionsUrl;
};

using namespace KSaneCore;

Skanpage::Skanpage(const QString &deviceName, const QUrl &dumpOptionsUrl, QObject *parent)
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

    d->m_dumpOptionsUrl = dumpOptionsUrl;
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
    d->m_scanArea = QRectF();

    Option *tlx = d->m_ksaneInterface.getOption(Interface::TopLeftXOption);
    Option *tly = d->m_ksaneInterface.getOption(Interface::TopLeftYOption);
    Option *brx = d->m_ksaneInterface.getOption(Interface::BottomRightXOption);
    Option *bry = d->m_ksaneInterface.getOption(Interface::BottomRightYOption);

    if (tlx && tly && brx && bry &&
        tlx->state() == Option::StateActive && tly->state() == Option::StateActive &&
        brx->state() == Option::StateActive && bry->state() == Option::StateActive
    ) {
        QVariant tlxMin = tlx->minimumValue(), tlyMin = tly->minimumValue();
        QVariant brxMax = brx->maximumValue(), bryMax = bry->maximumValue();
        if (tlxMin.isValid() && tlyMin.isValid() && brxMax.isValid() && bryMax.isValid()) {
            d->m_maximumScanArea.setCoords(tlxMin.toReal(), tlyMin.toReal(), brxMax.toReal(), bryMax.toReal());
            d->m_scanArea.setCoords(tlx->value().toReal() / d->m_maximumScanArea.width(),
                                    tly->value().toReal() / d->m_maximumScanArea.height(),
                                    brx->value().toReal() / d->m_maximumScanArea.width(),
                                    bry->value().toReal() / d->m_maximumScanArea.height());

            if (!d->m_scanAreaConnectionsDone) {
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
                d->m_scanAreaConnectionsDone = true;
            }
        }
        connect(tlx, &Option::optionReloaded, this, &Skanpage::setupScanningBounds, Qt::UniqueConnection);
        connect(tly, &Option::optionReloaded, this, &Skanpage::setupScanningBounds, Qt::UniqueConnection);
        connect(brx, &Option::optionReloaded, this, &Skanpage::setupScanningBounds, Qt::UniqueConnection);
        connect(bry, &Option::optionReloaded, this, &Skanpage::setupScanningBounds, Qt::UniqueConnection);
    }
    Q_EMIT scanAreaChanged(d->m_scanArea);
}

QRectF Skanpage::scanArea() const
{
    return d->m_scanArea;
}

void Skanpage::setScanArea(QRectF area)
{
    if (area == d->m_scanArea) return;
    if (d->m_ksaneInterface.getOption(Interface::TopLeftXOption) == NULL ||
        d->m_ksaneInterface.getOption(Interface::TopLeftYOption) == NULL ||
        d->m_ksaneInterface.getOption(Interface::BottomRightXOption) == NULL ||
        d->m_ksaneInterface.getOption(Interface::BottomRightYOption) == NULL) return;
    d->m_ksaneInterface.getOption(Interface::TopLeftXOption)->setValue(area.left() * d->m_maximumScanArea.width());
    d->m_ksaneInterface.getOption(Interface::TopLeftYOption)->setValue(area.top() * d->m_maximumScanArea.height());
    d->m_ksaneInterface.getOption(Interface::BottomRightXOption)->setValue(area.right() * d->m_maximumScanArea.width());
    d->m_ksaneInterface.getOption(Interface::BottomRightYOption)->setValue(area.bottom() * d->m_maximumScanArea.height());
}

Skanpage::ScanSplit Skanpage::scanSplit() const
{
    return d->m_scanSplit;
}

void Skanpage::setScanSplit(Skanpage::ScanSplit split)
{
    if (split != d->m_scanSplit) {
        d->m_scanSplit = split;
        Q_EMIT scanSplitChanged(d->m_scanSplit);
        if (split != ScanNotSplit) clearSubAreas();
    }
}

const QList<QRectF> &Skanpage::scanSubAreas()
{
    return d->m_scanSubAreas;
}

void Skanpage::clearSubAreas()
{
    if (!d->m_scanSubAreas.isEmpty()) {
        d->m_scanSubAreas.clear();
        Q_EMIT scanSubAreasChanged(d->m_scanSubAreas);
    }
}

void Skanpage::eraseSubArea(int index)
{
    d->m_scanSubAreas.removeAt(index);
    Q_EMIT scanSubAreasChanged(d->m_scanSubAreas);
}

bool Skanpage::appendSubArea(QRectF area)
{
    for (int i = 0; i < d->m_scanSubAreas.length(); i++) {
        if (area == d->m_scanSubAreas[i]) { // If the appended area is a duplicate
            return false;
        } else if (area.contains(d->m_scanSubAreas[i])) { // This area contains a smaller one within
            d->m_scanSubAreas.removeAt(i); i--; // Remove redundant areas
        } else if (area.intersects(d->m_scanSubAreas[i])) { // Avoid very similar (overlaping too much)
            // return; // To not allow any overlap
            QRectF intersect = area.intersected(d->m_scanSubAreas[i]);
            float overlapProportion = intersect.width()*intersect.height() / (area.width()*area.height());
            if (overlapProportion > 0.33) { d->m_scanSubAreas.removeAt(i); i--; }
        }
    }
    d->m_scanSubAreas.append(area);
    Q_EMIT scanSubAreasChanged(d->m_scanSubAreas);
    setScanSplit(Skanpage::ScanNotSplit); // Spliting and sub-areas... no
    return true;
}

void Skanpage::selectSubArea(int index)
{
    QRectF tmp = scanArea();
    setScanArea(scanSubAreas().at(index));
    eraseSubArea(index);
    appendSubArea(tmp);
}

QImage Skanpage::previewImage() const
{
    return d->m_previewImage;
}

bool Skanpage::previewImageAvailable() const
{
    return !d->m_previewImage.isNull();
}

void Skanpage::previewScan()
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
    if (!d->m_scanSubAreas.isEmpty()) {
        QRectF totalArea = d->m_scanArea; // Include last (unadded) area
        // This makes a rectangle that covers all the areas
        for (const QRectF& area : d->m_scanSubAreas) {
            totalArea = totalArea.united(area);
        }
        appendSubArea(d->m_scanArea); // Remember last area, for later use
        setScanArea(totalArea); // Scan all the needed area
    }
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
        return; // Do not save the preview to disk
    }
    if (d->m_scanSplit == ScanNotSplit && d->m_scanSubAreas.isEmpty()) {
        d->m_documentHandler.addImage(image);
        d->m_scannedImages++;
        return; // Regular scan ends here
    }
    auto applySubAreasToImage = [&]() {
        auto toOrigin = QTransform::fromTranslate(-d->m_scanArea.left(), -d->m_scanArea.top());
        auto toScale = QTransform::fromScale(image.width() / d->m_scanArea.width(), image.height() / d->m_scanArea.height());
        for (const QRectF& area : d->m_scanSubAreas) {
            QImage individualImage = image.copy(toScale.mapRect(toOrigin.mapRect(area)).toRect());
            d->m_documentHandler.addImage(individualImage);
            d->m_scannedImages++;
        }
    };
    if (!d->m_scanSubAreas.isEmpty()) { // There are sub-areas
        applySubAreasToImage();
        setScanArea(d->m_scanSubAreas.back()); // Leave scanArea as last selection
    } else {
        bool v = d->m_scanSplit == ScanIsSplitV;
        QRectF half = d->m_scanArea;
        if (v) half.setWidth(half.width()/2); else half.setHeight(half.height()/2);
        d->m_scanSubAreas.append(half); // Fake sub-areas, no need for notification
        if (v) half.moveRight(d->m_scanArea.right()); else half.moveBottom(d->m_scanArea.bottom());
        d->m_scanSubAreas.append(half);
        applySubAreasToImage();
    }
    clearSubAreas(); // The sub-areas last just one scan
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
    if (!d->m_dumpOptionsUrl.isEmpty()) {
        QJsonObject allScannerData;
        allScannerData[QLatin1StringView("scanner")] = d->m_ksaneInterface.scannerDeviceToJson();
        allScannerData[QLatin1StringView("options")] = d->m_ksaneInterface.scannerOptionsToJson();
        QString absolutePath;
        if (QDir::isAbsolutePath(d->m_dumpOptionsUrl.path())) {
            absolutePath = d->m_dumpOptionsUrl.toLocalFile();
        } else {
            absolutePath = QDir::current().absoluteFilePath(d->m_dumpOptionsUrl.toLocalFile());
        }
        QJsonDocument jsonDocument(allScannerData);
        QFile file(absolutePath);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        QTextStream out(&file);
        out << jsonDocument.toJson();
        qDebug() << QStringLiteral("The scanner device options have been saved to %1").arg(absolutePath);
        file.close();
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

    d->m_scanAreaConnectionsDone = false;
    setupScanningBounds();
    d->m_previewImage = QImage();
    Q_EMIT previewImageChanged(d->m_previewImage);

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
        if (variant.typeId() == QMetaType::QKeySequence) return variant.value<QKeySequence>();
        else if (variant.typeId() == QMetaType::QString) return variant.value<QString>();
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
        // Set the first, only, or empty shortcut. Passing a QKeySequence doesn't always work
        item->setProperty("shortcut", act->shortcut().toString(QKeySequence::PortableText));

        QList<QVariant> sequenceList; // To set the alternate shortcuts
        for (int i = 1; i < act->shortcuts().size(); i++) {
            sequenceList.append(act->shortcuts().at(i).toString(QKeySequence::PortableText));
        }
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

    if (d->m_scanIsPreview) { // imageReady didn't execute (there was an error)
        finishPreview(); // Restore options anyways
    }
    
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

#include "moc_Skanpage.cpp"
