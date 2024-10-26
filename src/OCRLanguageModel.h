/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef OCR_LANGUAGE_MODEL_H
#define OCR_LANGUAGE_MODEL_H

#include <QAbstractListModel>

struct LanguageItem {
    QString name;
    QString code;
    bool use;
};

class OCRLanguageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum LanguageModelRoles {
        NameRole = Qt::UserRole + 1,
        CodeRole,
        UseRole
    };

    explicit OCRLanguageModel(QObject *parent = nullptr);

    ~OCRLanguageModel() override;

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    void setLanguages(const std::vector<std::string> &availableLanguages);

    std::string getLanguagesString() const;

private:
    QList<LanguageItem> m_languages;
};

#endif // OCR_LANGUAGE_MODEL_H
