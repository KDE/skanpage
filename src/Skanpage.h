/* ============================================================
 *
 * Copyright (C) 2015 by Kåre Särs <kare.sars@iki .fi>
 * Copyright (C) 2021 by Alexander Stippich <a.stippich@gmx.net>
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

using namespace KSaneIface;

class Skanpage : public QObject
{
    Q_PROPERTY(int scanSizeIndex READ scanSizeIndex WRITE setScanSizeIndex NOTIFY scanSizeChanged)
    Q_PROPERTY(float scanDPI READ scanDPI WRITE setScanDPI NOTIFY scanDPIChanged)
    Q_PROPERTY(QStringList scanSizes READ scanSizes NOTIFY scanSizesChanged)
    Q_PROPERTY(bool colorMode READ colorMode WRITE setColorMode NOTIFY colorModeChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(DocumentModel *documentModel READ documentModel NOTIFY documentModelChanged)
    Q_PROPERTY(DevicesModel *devicesModel READ devicesModel NOTIFY devicesModelChanged)
    Q_PROPERTY(bool openedDevice READ openedDevice NOTIFY openedDeviceChanged)
    Q_PROPERTY(bool searchingForDevices READ searchingForDevices NOTIFY searchingForDevicesChanged)

    Q_PROPERTY(QString deviceVendor READ deviceVendor NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceModel READ deviceModel NOTIFY deviceInfoUpdated)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceInfoUpdated)
    
    Q_OBJECT

public:
    explicit Skanpage(const QString &deviceName, QObject *parent = nullptr);
    ~Skanpage();
    
    QString deviceVendor() const;
    QString deviceModel() const;
    QString deviceName() const;

    int scanSizeIndex() const;
    void setScanSizeIndex(int index);

    float scanDPI() const;
    void setScanDPI(float dpi);

    bool colorMode() const;
    void setColorMode(bool colorMode);

    QString errorMessage() const;
    const QStringList scanSizes() const;

    int progress() const;
    bool openedDevice() const;
    bool searchingForDevices() const;

    DocumentModel *documentModel() const;
    DevicesModel *devicesModel() const;

    Q_INVOKABLE void cancelScan();
    Q_INVOKABLE void reloadDevicesList();
    Q_INVOKABLE bool openDevice(const QString &deviceName);

Q_SIGNALS:
    void scanSizeChanged();
    void scanDPIChanged();
    void scanSizesChanged();
    void colorModeChanged();
    void progressChanged();
    void documentModelChanged();
    void devicesModelChanged();
    void errorMessageChanged();
    void openedDeviceChanged();
    void searchingForDevicesChanged();
    void deviceInfoUpdated();

public Q_SLOTS:
    void showAboutDialog();
    void loadScannerOptions();
    void saveScannerOptions();

    void startScan();
    void showScannerUI();

private Q_SLOTS:
    void imageReady(QByteArray &data, int width, int height, int bytesPerLine, int format);

    void defaultScannerOptions();

    void availableDevices(const QList<KSaneWidget::DeviceInfo> &deviceList);

    void alertUser(int type, const QString &strStatus);

    void progressUpdated(int progress);
    void scanDone(int status, const QString &strStatus);

private:
    void finishOpeningDevice(const QString &deviceName);
    int pageSizeToIndex(int id);
    void signalErrorMessage(const QString &text);

    std::unique_ptr<KSaneWidget> m_ksanew;
    std::unique_ptr<DocumentModel> m_docHandler;
    std::unique_ptr<DevicesModel> m_availableDevices;

    QMap<QString, QString> m_defaultScanOpts;
    QVector<QPageSize::PageSizeId> m_scanSizesEnum;
    QStringList m_scanSizesText;
    int m_scanSizeIndex;
    int m_progress;
    QString m_errorMessage;
    bool m_openedDevice;
    bool m_searchingForDevices;
};

#endif
