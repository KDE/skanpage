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

Item {
    id: doc

    property alias splitView: splitView

    focus: true
    clip: true

    readonly property string name: skanpage.documentModel.changed ? i18nc("Prefix for document name indicating an unsaved document", "* %1", skanpage.documentModel.name) : skanpage.documentModel.name;

    SystemPalette {
        id: palette
    }

    SplitView {
        id: splitView
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Kirigami.Separator {
            implicitWidth: 4 * Math.floor(Kirigami.Units.devicePixelRatio)
        }

        DocumentList {
            id: scrollView

            SplitView.fillHeight: true
            SplitView.preferredWidth: parent.width / 4
        }

        DocumentPage {
            id: activeDocument

            SplitView.fillWidth: true
            SplitView.fillHeight: true
                        visible: skanpage.progress >= 100
        }

        InProgressPage {
            id: inProgressImage
                        visible: skanpage.progress < 100
            SplitView.fillWidth: true
            SplitView.fillHeight: true
        }
    }
}
