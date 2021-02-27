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
#ifndef DocumentModel_H
#define DocumentModel_H

#include <QAbstractListModel>
#include <QFileInfo>
#include <QList>
#include <QObject>
#include <QPageSize>
#include <QStringList>
#include <QTemporaryFile>
#include <QUrl>

struct PageProperties {
    QTemporaryFile *temporaryFile;
    QPageSize pageSize;
    int dpi;
    int rotationAngle = 0;
};

class DocumentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool changed READ changed NOTIFY changedChanged)
    Q_PROPERTY(int activePageIndex READ activePageIndex WRITE setActivePageIndex NOTIFY activePageChanged)
    Q_PROPERTY(QUrl activePageSource READ activePageSource NOTIFY activePageChanged)
    Q_PROPERTY(int activePageRotation READ activePageRotation NOTIFY activePageChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_ENUMS(DocumentModelRoles)

public:
    enum DocumentModelRoles { ImageUrlRole = Qt::UserRole + 1 , RotationAngleRole};

    explicit DocumentModel(QObject *parent = nullptr);
    ~DocumentModel();

    const QString name() const;
    int activePageIndex() const;
    int activePageRotation() const;
    QUrl activePageSource() const;
    bool changed() const;
    
    void setActivePageIndex(int);

    void addImage(QTemporaryFile *tmpFile, QPageSize pageSize, int dpi);

    Q_INVOKABLE void clearData();

    Q_INVOKABLE void moveImage(int from, int to);
    
    Q_INVOKABLE void removeImage(int row);
    
    Q_INVOKABLE void rotateImage(int row, bool positiveDirection);

    Q_INVOKABLE void save(const QUrl &fileUrl);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void nameChanged();
    void changedChanged();
    void activePageChanged();
    void countChanged();

public Q_SLOTS:

private:
    void savePDF(const QString &name);
    void saveImage(const QFileInfo &fileInfo);

    QList<PageProperties> m_pages;
    QString m_name;
    bool m_changed;
    int m_activePageIndex = -1;
};

#endif
