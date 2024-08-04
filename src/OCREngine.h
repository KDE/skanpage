/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef OCR_ENGINE_H
#define OCR_ENGINE_H

#include "SkanpageUtils.h"

#include <QObject>
#include <QPainter>

#include <memory>

class OCREnginePrivate;
class QPdfWriter;
class OCRLanguageModel;

class OCREngine : public QObject
{
public:
    explicit OCREngine(QObject *parent = nullptr);
    ~OCREngine() override;

    void InitForOCR();

    bool available() const;

    OCRLanguageModel *languages() const;

    void OCRPage(QPdfWriter &writer, QPainter &painter, const SkanpageUtils::PageProperties &page);

    void setColor(QColor color);

private:
    std::unique_ptr<OCREnginePrivate> d;
};

#endif // #define OCR_ENGINE_H
