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
    id: inProgressPage

    Action {
        id: cancelAction
        icon.name: "window-close"
        text: i18n("Cancel")
        shortcut: "Esc"
        onTriggered: skanpage.cancelScan()
    }

    ColumnLayout {
        id: documentLayout

        anchors.fill: parent

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            InProgressPainter {
                id: inProgressImage
                anchors.fill: parent
                Component.onCompleted: inProgressImage.initialize(skanpage)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2

            ProgressBar {
                Layout.fillWidth: true
                value: skanpage.progress / 100
            }

            ToolButton {
                action: cancelAction
            }
        }
    }
}


