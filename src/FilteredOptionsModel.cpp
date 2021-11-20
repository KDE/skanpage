/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "FilteredOptionsModel.h"
#include "OptionsModel.h"

FilteredOptionsModel::FilteredOptionsModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

FilteredOptionsModel::~FilteredOptionsModel() = default;

bool FilteredOptionsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    const auto &state = sourceModel()->data(index, OptionsModel::StateRole).value<KSaneIface::KSaneOption::KSaneOptionState>();
    const auto &type = sourceModel()->data(index, OptionsModel::TypeRole).value<KSaneIface::KSaneOption::KSaneOptionType>();

     if (type == KSaneIface::KSaneOption::TypeValueList && sourceModel()->data(index, OptionsModel::ValueListRole).toList().length() <= 1) {
        return false;
    }
    
    if (!m_showAllOptions) {
        return sourceModel()->data(index, OptionsModel::FilterRole).toBool();
    }
    
    if (state != KSaneIface::KSaneOption::StateActive || type == KSaneIface::KSaneOption::TypeGamma || type == KSaneIface::KSaneOption::TypeDetectFail) {
        return false;
    }

    return true;
}

void FilteredOptionsModel::showAllOptions(bool show)
{
    if (m_showAllOptions != show) {
        m_showAllOptions = show;
        invalidateFilter();
    }
}
