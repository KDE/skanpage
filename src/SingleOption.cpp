/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

//KDE includes
#include <KSaneOption>

//own includes
#include "SingleOption.h"
#include "OptionsModel.h"
#include "skanpage_debug.h"

SingleOption::SingleOption(QObject *parent) : QObject(parent)
{
}

SingleOption::~SingleOption()
{
}

QVariant SingleOption::name() const
{
    if (mOption != nullptr) {
        return mOption->name();
    }
    return QVariant();
}

QVariant SingleOption::title() const
{
    if (mOption != nullptr) {
        return mOption->title();
    }
    return QVariant();
}

QVariant SingleOption::description() const
{
    if (mOption != nullptr) {
        return mOption->description();
    }
    return QVariant();
}

QVariant SingleOption::maximum() const
{
    if (mOption != nullptr) {
        return mOption->maximumValue();
    }
    return QVariant();
}

QVariant SingleOption::minimum() const
{
    if (mOption != nullptr) {
        return mOption->minimumValue();
    }
    return QVariant();
}

QVariant SingleOption::step() const
{
    if (mOption != nullptr) {
        return mOption->stepValue();
    }
    return 1;
}

QVariant SingleOption::valueList() const
{
    if (mOption != nullptr) {
        return mOption->valueList();
    }
    return QVariant();
}

QVariant SingleOption::unit() const
{
    if (mOption != nullptr) {
        return mOption->valueUnit();
    }
    return QVariant();
}

QVariant SingleOption::type() const
{
    if (mOption != nullptr) {
        return mOption->type();
    }
    return KSaneIface::KSaneOption::TypeDetectFail;
}

QVariant SingleOption::visible() const
{
    if (mOption != nullptr) {
        return mOption->state() == KSaneIface::KSaneOption::KSaneOptionState::StateActive;
    }
    return false;
}

QVariant SingleOption::value() const
{
    if (mOption != nullptr) {
        return mOption->value();
    }
    return QVariant();
}

bool SingleOption::setValue(QVariant &value)
{
    if (mOption != nullptr) {
        qCDebug(SKANPAGE_LOG()) << "SingleOption: Writing to option" << mOption->name() << value;
        return mOption->setValue(value);
    }
    return false;
}

void SingleOption::setOption(KSaneIface::KSaneOption *option)
{
    mOption = option;
    if (mOption != nullptr) {
        connect(option, &KSaneIface::KSaneOption::valueChanged, this, &SingleOption::valueChanged);
        connect(option, &KSaneIface::KSaneOption::optionReloaded, this, &SingleOption::optionReloaded);
    }
}

void SingleOption::clearOption()
{
    mOption = nullptr;
}
