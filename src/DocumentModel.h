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
#ifndef DocumentModel_H
#define DocumentModel_H

#include <QObject>
#include <QStringList>
#include <QAbstractListModel>
#include <QList>
#include <QTemporaryFile>

class DocumentModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(bool changed READ changed NOTIFY changedChanged)
    Q_ENUMS(DocumentModelRoles)

public:
    enum DocumentModelRoles {
        FileNameRole = Qt::UserRole+1
    };

    explicit DocumentModel(QObject *parent = nullptr);
    ~DocumentModel();

    const QString name() const;
    bool changed() const;

    void addImage(QTemporaryFile *tmpFile);
    
    Q_INVOKABLE void clearData();

    Q_INVOKABLE void moveImage(int from, int to);
    Q_INVOKABLE void removeImage(int row);

    Q_INVOKABLE void save(const QString &name, const QSizeF &pageSize, int dpi, const QString &title);

    Q_INVOKABLE bool fileExists(const QString &name) const;
    Q_INVOKABLE const QString toDisplayString(const QString &url) const;
    Q_INVOKABLE const QString upUrl(const QString &url) const;

public:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void nameChanged();
    void changedChanged();

public Q_SLOTS:

private:
    QList<QTemporaryFile *> m_tmpFiles;
    QString                 m_name;
    bool                    m_changed;

};

#endif
