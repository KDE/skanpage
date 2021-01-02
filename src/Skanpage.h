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

#ifndef Skanpage_h
#define Skanpage_h

#include <QObject>
#include <QMap>  // FIXME add this to KSaneWidget!!
#include <QString>
#include <QPageSize>
#include <QSizeF>

#include <KSaneWidget>

#include <memory>


class KAboutData;
class DocumentModel;

using namespace KSaneIface;

class Skanpage : public QObject
{
    Q_PROPERTY(int scanSizeIndex READ scanSizeIndex WRITE setScanSizeIndex NOTIFY scanSizeChanged)
    Q_PROPERTY(QStringList scanSizes READ scanSizes NOTIFY scanSizesChanged)
    Q_PROPERTY(QVariantList scanSizesF READ scanSizesF NOTIFY scanSizesChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(DocumentModel *documentModel READ documentModel NOTIFY documentModelChanged)
    
    Q_OBJECT

public:
    explicit Skanpage(const QString &device, QObject *parent = nullptr);
    ~Skanpage();
    void setAboutData(KAboutData *aboutData);

    int scanSizeIndex() const;
    void setScanSizeIndex(int index);
    
    QString errorMessage() const;
    const QStringList scanSizes() const;
   
    const QVariantList scanSizesF() const;

    int progress() const;
    
    DocumentModel *documentModel() const;

    Q_INVOKABLE const QSize windowSize() const;
    Q_INVOKABLE void saveWindowSize(const QSize &size);

    Q_INVOKABLE float scanDPI() const;
    Q_INVOKABLE bool setScanDPI(float dpi);

    Q_INVOKABLE void cancelScan();


Q_SIGNALS:
    void scanSizeChanged();
    void scanSizesChanged();
    void progressChanged();
    void documentModelChanged();
    void errorMessageChanged();

public Q_SLOTS:
    void showAboutDialog();
    void showHelp();
    void loadScannerOptions();
    void saveScannerOptions();

    void startScan();
    void showScannerUI();

private Q_SLOTS:
    void imageReady(QByteArray &, int, int, int, int);

    void defaultScannerOptions();

    void availableDevices(const QList<KSaneWidget::DeviceInfo> &deviceList);

    void alertUser(int type, const QString &strStatus);

    void progressUpdated(int progress);
    void scanDone(int status, const QString &strStatus);

private:
    int pageSizeToIndex(int id);
    void signalErrorMessage(const QString &text);

    KAboutData              *m_aboutData;
    std::unique_ptr<KSaneWidget> m_ksanew;
    std::unique_ptr<DocumentModel> m_docHandler;

    QString                  m_deviceName;
    QMap<QString, QString>   m_defaultScanOpts;
    QImage                   m_img;
    QByteArray               m_data;
    int                      m_width;
    int                      m_height;
    int                      m_bytesPerLine;
    int                      m_format;
    QVector<QPageSize::PageSizeId> m_scanSizesEnum;
    QVariantList             m_scanSizesF;
    QStringList              m_scanSizesText;
    int                      m_scanSizeIndex;
    int                      m_progress;
    QString                  m_errorMessage;
};

#endif

