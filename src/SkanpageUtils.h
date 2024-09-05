/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef SKANPAGE_UTILS_H
#define SKANPAGE_UTILS_H

#include <memory>

#include <QList>
#include <QPageSize>
#include <QTemporaryFile>

namespace SkanpageUtils
{

Q_NAMESPACE

enum MessageLevel {
    ErrorMessage,
    InformationMessage,
};

Q_ENUM_NS(MessageLevel)

enum FileType {
    EntireDocument,
    OCRDocument,
    PageSelection,
    SharingDocument,
};

Q_ENUM_NS(FileType)

struct PageProperties {
    std::shared_ptr<QTemporaryFile> temporaryFile;
    QPageSize pageSize;
    int dpi;
    int rotationAngle = 0;
};

typedef QList<PageProperties> DocumentPages;

bool operator==(const PageProperties &lhs, const PageProperties &rhs);
QDebug operator<<(QDebug d, const PageProperties &pageProperties);
}

Q_DECLARE_METATYPE(SkanpageUtils::MessageLevel)
Q_DECLARE_METATYPE(SkanpageUtils::PageProperties)
Q_DECLARE_METATYPE(SkanpageUtils::DocumentPages)

#endif // SKANPAGE_UTILS_H
