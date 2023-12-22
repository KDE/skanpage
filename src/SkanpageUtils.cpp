/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QDebug>

#include "SkanpageUtils.h"

bool SkanpageUtils::operator==(const PageProperties& lhs, const PageProperties& rhs)
{
    return lhs.dpi == rhs.dpi && lhs.pageSize == rhs.pageSize &&
    lhs.rotationAngle == rhs.rotationAngle && lhs.temporaryFile == rhs.temporaryFile;
}

QDebug SkanpageUtils::operator<<(QDebug d, const PageProperties& pageProperties)
{
    d << "DPI: " << pageProperties.dpi << "\n";
    d << "Page size: " << pageProperties.pageSize << "\n";
    d << "Rotation angle: " << pageProperties.rotationAngle << "\n";
    d << "Temporary file: " << pageProperties.temporaryFile.get() << "\n";
    return d;
}

#include "moc_SkanpageUtils.cpp"
