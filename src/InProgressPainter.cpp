/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "InProgressPainter.h"

#include <QPainter>

#include "skanpage_debug.h"

InProgressPainter::InProgressPainter(QQuickItem *parent) : QQuickPaintedItem(parent)
{
}

InProgressPainter::~InProgressPainter()
{
}

void InProgressPainter::initialize(Skanpage *skanpageApp)
{
    m_scanInterface = skanpageApp->ksaneInterface();
    connect(skanpageApp, &Skanpage::progressChanged, this, &InProgressPainter::updateImage);
}

void InProgressPainter::paint(QPainter *painter)
{
    if (m_scanInterface != nullptr && m_progress >= 0) {
        m_scanInterface->lockScanImage();

        const int imageHeight = m_scanInterface->scanImage()->height();
        const int imageWidth = m_scanInterface->scanImage()->width();
        const int itemHeight = height();
        const int itemWidth = width();

        double scaleHeight = static_cast<double>(itemHeight)/imageHeight;
        double scaleWidth = static_cast<double>(itemWidth)/imageWidth;
        double scale = qMin(scaleHeight, qMin(scaleWidth, 1.0));
        painter->drawImage(QRectF((itemWidth - scale * imageWidth)/2, (itemHeight - scale * imageHeight)/2, scale * imageWidth, scale * imageHeight), *m_scanInterface->scanImage());

        m_scanInterface->unlockScanImage();
    } else {
        painter->fillRect(QRect(0, 0, width(), height()), QColorConstants::Transparent);
    }
}

void InProgressPainter::updateImage(int progress)
{
    m_progress = progress;
    update();
}

#include "moc_InProgressPainter.cpp"
