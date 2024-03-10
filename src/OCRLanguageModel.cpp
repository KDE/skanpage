/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "OCRLanguageModel.h"

#include <QLocale>

OCRLanguageModel::OCRLanguageModel(QObject *parent) : QAbstractListModel(parent)
{
}

OCRLanguageModel::~OCRLanguageModel()
{
}

QHash<int, QByteArray> OCRLanguageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[CodeRole] = "code";
    roles[UseRole] = "use";
    return roles;
}

int OCRLanguageModel::rowCount(const QModelIndex &) const
{
    return m_languages.count();
}
    
QVariant OCRLanguageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_languages.size() || index.row() < 0) {
        return QVariant();
    }
    
    switch (role) {
    case NameRole:
        return m_languages.at(index.row()).name;
        break;
    case CodeRole:
        return m_languages.at(index.row()).code;
        break;
    case UseRole:
        return m_languages.at(index.row()).use;
        break;
    }
    return QVariant();
}

bool OCRLanguageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= 0 && index.row() < m_languages.size() && role == UseRole) {
        m_languages[index.row()].use = value.toBool();
        return true;
    }
    return false;
}

void OCRLanguageModel::setLanguages(const std::vector<std::string> &availableLanguages)
{
    beginResetModel();
    for (const auto &language : availableLanguages) {
        QString languageCode = QString::fromLocal8Bit(language.c_str());
        if (languageCode != QStringLiteral("osd")) {
            QLocale locale(QLocale::codeToLanguage(languageCode));
            m_languages.append({locale.nativeLanguageName(), languageCode, false});
        }
    }
    endResetModel();
}

std::string OCRLanguageModel::getLanguagesString() const
{
    std::string languageCodes;
    for (int i = 0; i < m_languages.size(); i++) {
        if (m_languages.at(i).use) {
            if (languageCodes.size() > 1) {
                languageCodes.append("+");
            }
            languageCodes.append(m_languages.at(i).code.toStdString());
        }
    }
    if (languageCodes.empty() && m_languages.size() != 0) {
        languageCodes.append(m_languages.constFirst().code.toStdString());
    }
    return languageCodes;
}

#include "moc_OCRLanguageModel.cpp"
