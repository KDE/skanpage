/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "SkanpageUtils.h"

bool SkanpageUtils::operator==(const PageProperties& lhs, const PageProperties& rhs)
{
    return lhs.dpi == rhs.dpi && lhs.pageSize == rhs.pageSize &&
    lhs.rotationAngle == rhs.rotationAngle && lhs.temporaryFile == rhs.temporaryFile;
}

