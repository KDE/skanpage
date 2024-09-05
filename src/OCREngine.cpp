/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "OCREngine.h"

#include "config-skanpage.h"

#include <KLocalizedString>
#include <QPdfWriter>

#if OCR_AVAILABLE
#include "OCRLanguageModel.h"
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#endif

#include "skanpage_debug.h"

class OCREnginePrivate
{
public:
#if OCR_AVAILABLE
    tesseract::TessBaseAPI m_tesseract;
    OCRLanguageModel m_languages;
    QColor m_penColor = Qt::transparent;
#endif
};

OCREngine::OCREngine(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<OCREnginePrivate>())
{
#if OCR_AVAILABLE
    if (d->m_tesseract.Init(nullptr, nullptr)) { // Use a default language, not necessarily English
        qCDebug(SKANPAGE_LOG) << "Failed tesseract OCR init";
        return;
    }
    d->m_tesseract.SetPageSegMode(tesseract::PSM_AUTO_OSD);
    std::vector<std::string> availableLanguages;
    d->m_tesseract.GetAvailableLanguagesAsVector(&availableLanguages);
    d->m_languages.setLanguages(availableLanguages);
#endif
}

OCREngine::~OCREngine()
{
}

bool OCREngine::available() const
{
    return OCR_AVAILABLE;
}

void OCREngine::InitForOCR()
{
#if OCR_AVAILABLE
    if (d->m_tesseract.Init(nullptr, d->m_languages.getLanguagesString().c_str())) {
        qCDebug(SKANPAGE_LOG) << "Failed tesseract OCR init";
        return;
    }
#endif
}

void OCREngine::setColor(QColor color)
{
#if OCR_AVAILABLE
    d->m_penColor = color;
#else
    Q_UNUSED(color)
#endif
}

OCRLanguageModel *OCREngine::languages() const
{
#if OCR_AVAILABLE
    return &d->m_languages;
#else
    return nullptr;
#endif
}

void OCREngine::OCRPage(QPdfWriter &writer, QPainter &painter, const SkanpageUtils::PageProperties &page)
{
#if OCR_AVAILABLE
    Pix *image = pixRead(page.temporaryFile->fileName().toStdString().c_str());
    d->m_tesseract.SetImage(image);
    d->m_tesseract.SetSourceResolution(page.dpi);
    d->m_tesseract.Recognize(nullptr);

    tesseract::ResultIterator *it = d->m_tesseract.GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    painter.setPen(d->m_penColor);

    int baseX1, baseY1, baseX2, baseY2, fontID, pointSize;
    bool bold, italic, underlined, monospace, serif, smallcaps;
    tesseract::Orientation orientation;
    tesseract::WritingDirection direction;
    tesseract::TextlineOrder order;
    float deskew_angle;

    QFont font;
    /* Currently, each word extracted from tesseract is printed
     * at it coordinates with transparent color.
     * We stretch each word to the bounding box since the actual used
     * fonts may be different. */
    const QTransform oldTransformation = painter.transform();
    if (it != nullptr) {
        do {
            if (it->Empty(level)) {
                continue;
            }
            const char *word = it->GetUTF8Text(level);
            it->Baseline(level, &baseX1, &baseY1, &baseX2, &baseY2);
            it->WordFontAttributes(&bold, &italic, &underlined, &monospace, &serif, &smallcaps, &pointSize, &fontID);
            it->Orientation(&orientation, &direction, &order, &deskew_angle);
            /* Font attributes other than pointSize do not work
             * https://github.com/tesseract-ocr/tesseract/issues/1074 */
            if (pointSize <= 0) {
                pointSize = 8;
            }
            font.setStretch(100);
            font.setPointSize(pointSize);
            QFontMetrics metrics(font, &writer);
            const QString text = QString::fromUtf8(word);
            const QRect textBounds = metrics.boundingRect(text);
            int stretch = 100;
            QTransform transformation;
            transformation.translate(baseX1, baseY1);
            if (orientation == tesseract::ORIENTATION_PAGE_RIGHT) {
                transformation.rotate(90);
                stretch = static_cast<double>(baseY2 - baseY1) / textBounds.width() * 100;
            } else if (orientation == tesseract::ORIENTATION_PAGE_LEFT) {
                transformation.rotate(270);
                stretch = static_cast<double>(baseY1 - baseY2) / textBounds.width() * 100;
            } else if (orientation == tesseract::ORIENTATION_PAGE_DOWN) {
                transformation.rotate(180);
                stretch = static_cast<double>(baseX1 - baseX2) / textBounds.width() * 100;
            } else {
                stretch = static_cast<double>(baseX2 - baseX1) / textBounds.width() * 100;
            }
            if (stretch <= 0) {
                stretch = 100;
            }
            transformation.rotate(deskew_angle);
            font.setStretch(stretch);
            transformation.translate(-baseX1, -baseY1);
            painter.setFont(font);
            painter.setTransform(transformation, true);
            painter.drawText(baseX1, baseY1, text);
            delete[] word;
            painter.setTransform(oldTransformation);
        } while (it->Next(level));
    }
#else
    Q_UNUSED(writer)
    Q_UNUSED(painter)
    Q_UNUSED(page)
#endif
}
