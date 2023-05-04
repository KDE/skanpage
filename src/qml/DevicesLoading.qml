/**
 * SPDX-FileCopyrightText: 2020 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami
import org.kde.skanpage 1.0

Item {

    ColumnLayout {
        anchors.centerIn: parent

        BusyIndicator {
            running: skanpage.applicationState === Skanpage.SearchingForDevices

            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        Kirigami.PlaceholderMessage {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            text: xi18nc("@info", "Searching for available devices.")
        }
    }
}
