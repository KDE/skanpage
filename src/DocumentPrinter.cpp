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

void DocumentPrinter::printDocument(const SkanpageUtils::DocumentPages &document)
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

    if (printDialog.exec() == QDialog::Accepted) {
        m_future = QtConcurrent::run(&DocumentPrinter::print, this, document);
    }
}

void DocumentPrinter::print(const SkanpageUtils::DocumentPages &document)
{
    QPainter painter(m_printer.get());
    QRect rect = painter.viewport();
    for (int i = 0; i < document.count(); ++i) {
        if (i != 0) {
            m_printer->newPage();
        }
        int rotationAngle = document.at(i).rotationAngle;
        QImage pageImage(document.at(i).temporaryFile->fileName());
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
