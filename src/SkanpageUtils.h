/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef SkanpageUtils_H
#define SkanpageUtils_H

#include <memory>

#include <QPageSize>
#include <QTemporaryFile>
#include <QList>

namespace SkanpageUtils {

    Q_NAMESPACE

    enum MessageLevel {
        ErrorMessage,
        InformationMessage,
    };

    Q_ENUM_NS(MessageLevel)

    struct PageProperties {
        std::shared_ptr<QTemporaryFile> temporaryFile;
        QPageSize pageSize;
        int dpi;
        int rotationAngle = 0;
    };

    typedef QList<PageProperties> DocumentPages;

    bool operator==(const PageProperties& lhs, const PageProperties& rhs);
}

Q_DECLARE_METATYPE(SkanpageUtils::PageProperties)
Q_DECLARE_METATYPE(SkanpageUtils::DocumentPages)

#endif // SkanpageUtils_H
