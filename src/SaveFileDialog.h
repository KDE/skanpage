/* ============================================================
 *
 * Copyright (C) 2016 by Kåre Särs <kare.sars@iki .fi>
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
#ifndef SaveFileDialog_H
#define SaveFileDialog_H

#include <QObject>
#include <QString>

class SaveFileDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setFilters NOTIFY nameFiltersChanged)

public:
    explicit SaveFileDialog(QObject *parent = nullptr);
    ~SaveFileDialog();

    const QString title() const;
    void setTitle(const QString &title);

    const QString fileUrl() const;
    void setFileUrl(const QString &fileUrl);

    const QStringList nameFilters() const;
    void setFilters(const QStringList &nameFilters);

    Q_INVOKABLE const QString toFileName(const QString &url) const;
    Q_INVOKABLE const QString toBaseName(const QString &url) const;
    Q_INVOKABLE const QString documentsDir() const;

public Q_SLOTS:
    Q_INVOKABLE void open();
    Q_INVOKABLE void open(const QString &title, const QString &fileUrl, const QString &nameFilters);

Q_SIGNALS:
    void titleChanged();
    void fileUrlChanged();
    void nameFiltersChanged();

    void accepted();
    void rejected();


private:
    QString m_title;
    QString m_fileUrl;
    QString m_nameFilters;
};

#endif
