/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "DocumentPrinter.h"

#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>

#include <QTransform>
#include <QtConcurrent>

#include <KLocalizedString>

#include "skanpage_debug.h"

DocumentPrinter::DocumentPrinter(QObject *parent)
    : QObject(parent)
    , m_printer(std::make_unique<QPrinter>())
{
}

DocumentPrinter::~DocumentPrinter()
{
}

void DocumentPrinter::printDocument(const SkanpageUtils::DocumentPages &document, const int currentIndex)
{
    if (document.isEmpty()) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Nothing to print."));
        return;
    }
    if (m_future.isRunning()) {
        Q_EMIT showUserMessage(SkanpageUtils::ErrorMessage, i18n("Previous printing operation still in progress."));
        return;
    }

    QPrintDialog printDialog(m_printer.get());
    printDialog.setMinMax(1, document.count());
    printDialog.setFromTo(1, document.count());
    printDialog.setOption(QPrintDialog::PrintCurrentPage);
    printDialog.setOption(QPrintDialog::PrintPageRange, false);
    printDialog.setOption(QPrintDialog::PrintToFile, false);
    m_printer->setFromTo(1, document.count());
    if (printDialog.exec() == QDialog::Accepted) {
        m_future = QtConcurrent::run(&DocumentPrinter::print, this, document, currentIndex);
    }
}

void DocumentPrinter::print(const SkanpageUtils::DocumentPages &document, const int currentIndex)
{
    QList<int> pagesToPrint;

    if (m_printer->printRange() == QPrinter::CurrentPage) {
        pagesToPrint << currentIndex;
        /* Always print all pages and rely on server-side page range feature in QPrintDialog with CUPS printing.
        * Necessary because there is a bug in the current implementation with application-based page ranges.
        * It sets the print range inconsistently and hence we may apply mulitple page ranges twice and causing unexpected results.
        } else if (m_printer->printRange() == QPrinter::PageRange) {
            const QList<QPageRanges::Range> rangesList = m_printer->pageRanges().toRangeList();
            for (int i = 0; i < rangesList.count(); ++i) {
                QPageRanges::Range range = rangesList.at(i);
                pagesToPrint.reserve(pagesToPrint.count() + range.to - range.from + 1);
                for (int j = range.from - 1; j < range.to; ++j) {
                    pagesToPrint.append(j);
                }
            } */
    } else { // all pages
        pagesToPrint.reserve(document.count());
        for (int i = 0; i < document.count(); ++i) {
            pagesToPrint.append(i);
        }
    }

    if (m_printer->pageOrder() == QPrinter::LastPageFirst) {
        std::reverse(pagesToPrint.begin(), pagesToPrint.end());
    }

    QPainter painter(m_printer.get());
    QRect rect = painter.viewport();
    for (int i = 0; i < pagesToPrint.count(); ++i) {
        const int pageNumber = pagesToPrint.at(i);
        if (pageNumber >= document.count()) {
            qCDebug(SKANPAGE_LOG) << "Skip printing page" << pageNumber + 1 << " as it does not exist";
        } else {
            qCDebug(SKANPAGE_LOG) << "Printing page" << pageNumber + 1;
        }
        if (i != 0) {
            m_printer->newPage();
        }
        int rotationAngle = document.at(pageNumber).rotationAngle;
        QImage pageImage(document.at(pageNumber).temporaryFile->fileName());
        if (rotationAngle != 0) {
            pageImage = pageImage.transformed(QTransform().rotate(rotationAngle));
        }

        QSize size = pageImage.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(pageImage.rect());
        painter.drawImage(0, 0, pageImage);
    }

    Q_EMIT showUserMessage(SkanpageUtils::InformationMessage, i18n("Document sent to printer."));
}

#include "moc_DocumentPrinter.cpp"
