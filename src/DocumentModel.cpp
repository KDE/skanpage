/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DocumentModel.h"

#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QPdfWriter>
#include <QUrl>
#include <QTransform>
#include <QTemporaryFile>

#include "skanpage_debug.h"
#include <KLocalizedString>

DocumentModel::DocumentModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_name(i18n("New document"))
{
}

DocumentModel::~DocumentModel()
{
    for (auto &page : qAsConst(m_pages)) {
        delete page.temporaryFile;
    }
}

const QString DocumentModel::name() const
{
    return m_name;
}

bool DocumentModel::changed() const
{
    return m_changed;
}

int DocumentModel::activePageIndex() const
{
    return m_activePageIndex;
}

int DocumentModel::activePageRotation() const
{
    if (m_activePageIndex >= 0 && m_activePageIndex < rowCount()) { 
        return m_pages.at(m_activePageIndex).rotationAngle;
    }
    return 0;
}

QUrl DocumentModel::activePageSource() const
{
    if (m_activePageIndex >= 0 && m_activePageIndex < rowCount()) {
        return QUrl::fromLocalFile(m_pages.at(m_activePageIndex).temporaryFile->fileName());
    }
    return QUrl();
}

void DocumentModel::setActivePageIndex(int newIndex)
{
    if (newIndex != m_activePageIndex) {
        m_activePageIndex = newIndex;
        Q_EMIT activePageChanged();
    }
}

void DocumentModel::save(const QUrl &fileUrl)
{
    if (fileUrl.isEmpty() || m_pages.isEmpty()) {
        return;
    }
    qCDebug(SKANPAGE_LOG) << QStringLiteral("Saving document to") << fileUrl;

    const auto localFile = fileUrl.toLocalFile();
    const auto fileInfo = QFileInfo(localFile);
    const auto fileSuffix = fileInfo.suffix();

    qCDebug(SKANPAGE_LOG) << QStringLiteral("Selected file suffix is") << fileSuffix;

    if (fileSuffix == QLatin1String("pdf") || fileSuffix.isEmpty()) {
        savePDF(localFile);
    } else {
        saveImage(fileInfo);
    }

    if (m_changed) {
        m_changed = false;
        Q_EMIT changedChanged();
    }

    if (m_name != fileInfo.fileName()) {
        m_name = fileInfo.fileName();
        Q_EMIT nameChanged();
    }
}

void DocumentModel::savePDF(const QString &name)
{
    QPdfWriter writer(name);
    QPainter painter;
    int rotationAngle;
    
    for (int i = 0; i < m_pages.count(); ++i) {
        writer.setResolution(m_pages.at(i).dpi);
        writer.setPageSize(m_pages.at(i).pageSize);
        writer.setPageMargins(QMarginsF(0, 0, 0, 0));
        rotationAngle = m_pages.at(i).rotationAngle;

        if (rotationAngle == 90 ||  rotationAngle == 270) {
            writer.setPageOrientation(QPageLayout::Landscape);
        } else {
            writer.setPageOrientation(QPageLayout::Portrait);   
        }
        writer.newPage();
        
        if (i == 0) {
            painter.begin(&writer);
        }
        
        QImage pageImage(m_pages.at(i).temporaryFile->fileName());
        if (rotationAngle != 0) {
            pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
        }
        
        QSize targetSize(writer.width(), writer.height());
        pageImage = pageImage.scaled(targetSize, Qt::KeepAspectRatio, Qt::FastTransformation);
        painter.drawImage(pageImage.rect(), pageImage, pageImage.rect());
    }
}

void DocumentModel::saveImage(const QFileInfo &fileInfo)
{
    const int count = m_pages.count();
    QImage pageImage;
    QString fileName;

    if (count == 1) {
        pageImage.load(m_pages.at(0).temporaryFile->fileName());
        fileName = fileInfo.absoluteFilePath();
        const int rotationAngle = m_pages.at(0).rotationAngle;
        if (rotationAngle != 0) {
            pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
        }
        pageImage.save(fileName, fileInfo.suffix().toLocal8Bit().constData());
    } else {
        for (int i = 0; i < count; ++i) {
            pageImage.load(m_pages.at(i).temporaryFile->fileName());
            const int rotationAngle = m_pages.at(i).rotationAngle;
            if (rotationAngle != 0) {
                pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
            }
            fileName =
                QStringLiteral("%1/%2%3.%4").arg(fileInfo.absolutePath(), fileInfo.baseName(), QLocale().toString(i).rightJustified(4, QLatin1Char('0')), fileInfo.suffix());
            pageImage.save(fileName, fileInfo.suffix().toLocal8Bit().constData());
        }
    }
}

void DocumentModel::addImage(const QImage &image, const int dpi)
{
    const double conversionFactorMM = static_cast<double>(dpi) / 25.4;
    QPageSize pageSize = QPageSize(QSizeF(image.width() / conversionFactorMM, image.height()/ conversionFactorMM), QPageSize::Millimeter);
    QTemporaryFile *tempImageFile = new QTemporaryFile();
    tempImageFile->open();
    if (image.save(tempImageFile, "PNG")) {
        qCDebug(SKANPAGE_LOG) << "Adding new image file" << tempImageFile << " with pageSize" << pageSize << "and resolution " << dpi << "dpi";
    } else {
         Q_EMIT showUserMessage(Skanpage::ErrorMessage, i18n("Failed to save image"));
    }
    tempImageFile->close();
    beginInsertRows(QModelIndex(), m_pages.count(), m_pages.count());
    m_pages.append({tempImageFile, pageSize, dpi});
    endInsertRows();
   
    Q_EMIT countChanged();
    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    } 
    
    m_activePageIndex = m_pages.count() - 1;
    Q_EMIT activePageChanged();
}

void DocumentModel::moveImage(int from, int to)
{
    int add = 0;
    if (from == to) {
        return;
    }
    if (to > from) {
        add = 1;
    }
    if (from < 0 || from >= m_pages.count()) {
        return;
    }
    if (to < 0 || to >= m_pages.count()) {
        return;
    }

    bool ok = beginMoveRows(QModelIndex(), from, from, QModelIndex(), to + add);
    if (!ok) {
        qCDebug(SKANPAGE_LOG) << "Failed to move" << from << to << add << m_pages.count();
        return;
    }
    m_pages.move(from, to);
    endMoveRows();

    if (m_activePageIndex == from) {
        m_activePageIndex = to;
    } else if (m_activePageIndex == to) {
        m_activePageIndex = from;
    }
    Q_EMIT activePageChanged();
    
    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }
}

void DocumentModel::rotateImage(int row, bool positiveDirection)
{
    if (row < 0 || row >= rowCount()) {
        return;
    }
    int rotationAngle = m_pages.at(row).rotationAngle;
    if (positiveDirection) {
        rotationAngle += 90;
    } else {
        rotationAngle -= 90;
    }
    if (rotationAngle < 0) {
        rotationAngle = rotationAngle + 360;
    } else if (rotationAngle >= 360) {
        rotationAngle = rotationAngle - 360;
    }
    m_pages[row].rotationAngle = rotationAngle;
    if (row == m_activePageIndex) {
        Q_EMIT activePageChanged();
    }
    Q_EMIT dataChanged(index(row,0), index(row,0), {RotationAngleRole});
}

void DocumentModel::removeImage(int row)
{
    if (row < 0 || row >= m_pages.count()) {
        return;
    }
    
    beginRemoveRows(QModelIndex(), row, row);
    delete m_pages.at(row).temporaryFile;
    m_pages.removeAt(row);
    endRemoveRows();
        
    if (row < m_activePageIndex) {
        m_activePageIndex -= 1;
    } else if (m_activePageIndex >= m_pages.count()) {
        m_activePageIndex = m_pages.count() - 1;
    }
    Q_EMIT activePageChanged();
    
    Q_EMIT countChanged();

    if (!m_changed) {
        m_changed = true;
        Q_EMIT changedChanged();
    }
}

QHash<int, QByteArray> DocumentModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageUrlRole] = "imageUrl";
    roles[RotationAngleRole] = "rotationAngle";
    return roles;
}

int DocumentModel::rowCount(const QModelIndex &) const
{
    return m_pages.count();
}

QVariant DocumentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_pages.size() || index.row() < 0) {
        return QVariant();
    }

    switch (role) {
    case ImageUrlRole:
        return QUrl::fromLocalFile(m_pages.at(index.row()).temporaryFile->fileName());
    case RotationAngleRole:
        return m_pages.at(index.row()).rotationAngle;
    }
    return QVariant();
}

void DocumentModel::clearData()
{
    beginResetModel();
    m_pages.clear();
    m_activePageIndex = -1;
    endResetModel();
    Q_EMIT countChanged();

    Q_EMIT countChanged();
    
    if (m_changed) {
        m_changed = false;
        Q_EMIT changedChanged();
    }
}
