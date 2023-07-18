/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef SKANPAGE_H
#define SKANPAGE_H

#include <QObject>
#include <QString>
#include <QImage>

#include <KActionCollection>

#include <Interface>
#include <DeviceInformation>

#include <memory>

#include "SkanpageUtils.h"
#include "skanpage_config.h"
#include "skanpage_state.h"

class DocumentModel;
class DevicesModel;
class FormatModel;
class FilteredOptionsModel;
class OCRLanguageModel;
class SkanpagePrivate;

class Skanpage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int countDown READ countDown NOTIFY countDownChanged)
    Q_PROPERTY(DocumentModel *documentModel READ documentModel CONSTANT)
    Q_PROPERTY(DevicesModel *devicesModel READ devicesModel CONSTANT)
    Q_PROPERTY(FormatModel *formatModel READ formatModel CONSTANT)
    Q_PROPERTY(FilteredOptionsModel *optionsModel READ optionsModel CONSTANT)
    Q_PROPERTY(OCRLanguageModel *languageModel READ languageModel CONSTANT)
    Q_PROPERTY(SkanpageConfiguration *configuration READ configuration CONSTANT)
    Q_PROPERTY(SkanpageState *stateConfiguration READ stateConfiguration CONSTANT)
    Q_PROPERTY(ApplicationState applicationState READ applicationState NOTIFY applicationStateChanged)

    Q_PROPERTY(QString deviceVendor READ deviceVendor NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceModel READ deviceModel NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceInfoUpdated)

    Q_PROPERTY(QRectF scanArea READ scanArea WRITE setScanArea NOTIFY scanAreaChanged)
    Q_PROPERTY(ScanSplit scanSplit READ scanSplit WRITE setScanSplit NOTIFY scanSplitChanged)
    Q_PROPERTY(QList<QRectF> scanSubAreas READ scanSubAreas NOTIFY scanSubAreasChanged)
    Q_PROPERTY(QImage previewImage READ previewImage NOTIFY previewImageChanged)

public:

    enum ApplicationState {
        ScanInProgress,
        ReadyForScan,
        DeviceSelection,
        SearchingForDevices,
        NoDeviceOpened
    };
    Q_ENUM(ApplicationState)

    enum ScanSplit {
        ScanNotSplit,
        ScanIsSplitH,
        ScanIsSplitV
    };
    Q_ENUM(ScanSplit)

    explicit Skanpage(const QString &deviceName, QObject *parent = nullptr);
    ~Skanpage();

    QString deviceVendor() const;
    QString deviceModel() const;
    QString deviceName() const;

    QRectF scanArea() const;
    void setScanArea(QRectF area);
    ScanSplit scanSplit() const;
    void setScanSplit(ScanSplit split);
    const QList<QRectF> &scanSubAreas();
    QImage previewImage() const;

    int progress() const;
    int countDown() const;
    ApplicationState applicationState() const;

    DocumentModel *documentModel() const;
    DevicesModel *devicesModel() const;
    FormatModel *formatModel() const;
    FilteredOptionsModel *optionsModel() const;
    OCRLanguageModel *languageModel() const;
    KSaneCore::Interface *ksaneInterface() const;
    SkanpageConfiguration *configuration() const;
    SkanpageState *stateConfiguration() const;

    Q_INVOKABLE void preview();
    Q_INVOKABLE void startScan();
    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void reloadDevicesList();
    Q_INVOKABLE bool openDevice(const QString &deviceName, const QString &deviceVendor = QString(), const QString &deviceModel = QString());    
    Q_INVOKABLE bool OCRavailable() const;
    Q_INVOKABLE void print();
    Q_INVOKABLE void registerAction(QObject* item, QObject* shortcuts, const QString &iconText);
    Q_INVOKABLE void showShortcutsDialog();
    Q_INVOKABLE void clearSubAreas();
    Q_INVOKABLE void eraseSubArea(int index);
    Q_INVOKABLE bool appendSubArea(QRectF area);
    Q_INVOKABLE void selectSubArea(int index);

Q_SIGNALS:
    void progressChanged(int progress);
    void countDownChanged(int remainingSeconds);
    void optionsChanged();
    void applicationStateChanged(ApplicationState state);
    void deviceInfoUpdated();
    void scanAreaChanged(const QRectF &area);
    void scanSplitChanged(ScanSplit split);
    void scanSubAreasChanged(const QList<QRectF> &subAreas);
    void previewImageChanged(const QImage& preview);
    void newUserMessage(const QVariant &level, const QVariant &message);

private Q_SLOTS:

    void imageReady(const QImage &image);
    void availableDevices(const QList<KSaneCore::DeviceInformation *> &deviceList);
    void showKSaneMessage(KSaneCore::Interface::ScanStatus type, const QString &strStatus);
    void showUserMessage(SkanpageUtils::MessageLevel level, const QString &strStatus);
    void progressUpdated(int progress);
    void batchModeCountDown(int remainingSeconds);
    void scanningFinished(KSaneCore::Interface::ScanStatus status, const QString &strStatus);
    void imageTemporarilySaved();

private:
    void finishOpeningDevice(const QString &deviceName, const QString &deviceVendor, const QString &deviceModel);
    void loadScannerOptions();
    void saveScannerOptions();
    void checkFinish();
    void finishPreview();
    void setupScanningBounds();
    void signalErrorMessage(const QString &text);

    std::unique_ptr<SkanpagePrivate> d;
};

#endif // SKANPAGE_H
