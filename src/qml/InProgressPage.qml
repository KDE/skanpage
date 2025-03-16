/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.1

import org.kde.kirigami as Kirigami
import org.kde.skanpage 1.0

Item {
    id: inProgressPage

    Kirigami.PlaceholderMessage {
        id: countDownTimerMessage

        anchors.centerIn: parent
        visible: skanpage.countDown > 0

        icon.name: "clock"

        text: xi18nc("Countdown string with time given in seconds", "Next scan starts in<nl/>%1 s", skanpage.countDown)
    }

    ColumnLayout {
        anchors.centerIn: parent

        visible: skanpage.progress < 0

        BusyIndicator {
            running: parent.visible

            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        Kirigami.PlaceholderMessage {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: i18nc("@info", "Scan in progress…")
        }
    }

    ColumnLayout {
        id: documentLayout
        visible: skanpage.countDown <= 0 && skanpage.progress >= 0
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

        ProgressBar {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            value: skanpage.progress / 100
        }
    }
}


