/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami
import org.kde.skanpage 1.0

Item {
    id: doc

    readonly property string name: skanpage.documentModel.changed ? i18nc("Prefix for document name indicating an unsaved document", "* %1", skanpage.documentModel.name) : skanpage.documentModel.name;
    property alias splitView: splitView
    signal saveSinglePage(int pageNumber)

    focus: true
    clip: true

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Item {
            implicitWidth: Kirigami.Units.smallSpacing

            Kirigami.Separator {
                height: parent.height
                width: Kirigami.Units.smallSpacing / 2
                anchors.centerIn: parent
            }
        }

        DocumentList {
            id: scrollView

            SplitView.fillHeight: true
            SplitView.preferredWidth: parent.width / 4
            onSaveSinglePage: doc.saveSinglePage(pageNumber)
        }

        DocumentPage {
            id: activeDocument

            SplitView.fillWidth: true
            SplitView.fillHeight: true

            visible: skanpage.applicationState == Skanpage.ReadyForScan
            onSaveSinglePage: doc.saveSinglePage(pageNumber)
        }

        InProgressPage {
            id: inProgressImage

            SplitView.fillWidth: true
            SplitView.fillHeight: true

            visible: skanpage.applicationState == Skanpage.ScanInProgress
        }
    }
}
