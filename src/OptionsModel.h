/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef OPTIONSMODEL_H
#define OPTIONSMODEL_H

#include <QObject>
#include <QList>
#include <QAbstractListModel>

#include <memory>

#include <KSaneOption>

class OptionsModelPrivate;

class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

    Q_PROPERTY(bool isModified READ isModified NOTIFY isModifiedChanged)

public:
    enum OptionsModelRoles {
        NameRole = Qt::UserRole + 1,
        TitleRole,
        DescriptionRole,
        ValueRole,
        MaximumValueRole,
        MinimumValueRole,
        StepValueRole,
        ValueListRole,
        UnitRole,
        TypeRole,
        VisibleRole,
    };

    explicit OptionsModel(QObject *parent = nullptr);

    ~OptionsModel();

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = ValueRole) override;

    void setOptionsList(const QList<KSaneIface::KSaneOption *> optionsList);

    bool isModified() const;

    Q_INVOKABLE void resetOptionsValues();

    Q_INVOKABLE void saveOptionsValues();

    void clearOptions();

Q_SIGNALS:

    void rowCountChanged();

    void isModifiedChanged();

private:

    std::unique_ptr<OptionsModelPrivate> d;
};

#endif // OPTIONSMODEL_H
