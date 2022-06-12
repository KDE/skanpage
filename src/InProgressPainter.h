/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef IN_PROGRESS_PAINTER_H
#define IN_PROGRESS_PAINTER_H

#include <QQuickPaintedItem>

#include "Skanpage.h"

class InProgressPainter : public QQuickPaintedItem
{
    Q_OBJECT

public:
    InProgressPainter(QQuickItem *parent = nullptr);
    ~InProgressPainter();

    void paint(QPainter *painter) override;
    Q_INVOKABLE void initialize(Skanpage *skanpageApp);

private Q_SLOTS:
    void updateImage(int progress);

private:
    KSaneCore::Interface *m_scanInterface = nullptr;
    int m_progress = 0;
};

#endif // IN_PROGRESS_PAINTER_H
