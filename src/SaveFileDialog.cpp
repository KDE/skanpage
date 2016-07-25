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
#include "SaveFileDialog.h"
#include <QFileDialog>
#include <QDebug>
#include <QStandardPaths>

SaveFileDialog::SaveFileDialog(QObject *parent)
:QObject(parent)
,m_title(QStringLiteral("Save File"))
,m_nameFilters(QStringLiteral("*"))
{}
SaveFileDialog::~SaveFileDialog() {}

const QString SaveFileDialog::title() const { return m_title; }
void SaveFileDialog::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        emit titleChanged();
    }
}

const QString SaveFileDialog::fileUrl() const { return m_fileUrl; }
void SaveFileDialog::setFileUrl(const QString &fileUrl)
{
    if (fileUrl != m_fileUrl) {
        m_fileUrl = fileUrl;
        emit fileUrlChanged();
    }
}

const QStringList SaveFileDialog::nameFilters() const { return m_nameFilters.split(QStringLiteral(";;")); }
void SaveFileDialog::setFilters(const QStringList &nameFilters)
{
    QString fList = nameFilters.join(QStringLiteral(";;"));
    if (m_nameFilters != fList) {
        m_nameFilters = fList;
        emit nameFiltersChanged();
    }
}

void SaveFileDialog::open()
{
    open(m_title, m_fileUrl, m_nameFilters);
}

void SaveFileDialog::open(const QString &title, const QString &fileUrl, const QString &nameFilters)
{

    m_fileUrl = QFileDialog::getSaveFileName(nullptr, title, fileUrl, nameFilters);

    emit fileUrlChanged();
    if (!m_fileUrl.isEmpty()) {

        emit accepted();
    }
    else {
        emit rejected();
    }
}

const QString SaveFileDialog::toFileName(const QString &url) const
{
    return QFileInfo(url).fileName();
}

const QString SaveFileDialog::toBaseName(const QString &url) const
{
    return QFileInfo(url).baseName();
}

const QString SaveFileDialog::documentsDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
}

