/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef FILTERED_OPTIONS_MODEL_H
#define FILTERED_OPTIONS_MODEL_H

#include <QSortFilterProxyModel>

class FilteredOptionsModel : public QSortFilterProxyModel
{

    Q_OBJECT

public:

    explicit FilteredOptionsModel(QObject *parent = nullptr);

    ~FilteredOptionsModel() override;

    Q_INVOKABLE void showAllOptions(bool show);

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_showAllOptions = true;
};

#endif // FILTERED_OPTIONS_MODEL_H
