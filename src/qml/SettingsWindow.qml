/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import @QTQUICK_DIALOGS_IMPORT@

import org.kde.kirigami 2.20 as Kirigami
import org.kde.skanpage 1.0

Window {
    id: settingsWindow

    title: i18n("Configure")
    color: Kirigami.Theme.backgroundColor

    flags: Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint
        | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint

    minimumWidth: Kirigami.Units.gridUnit * 30
    minimumHeight: Kirigami.Units.gridUnit * 15

    Kirigami.FormLayout {
        anchors.fill: parent
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.rightMargin: Kirigami.Units.gridUnit

        ButtonGroup { id: allDevicesGroup }

        RadioButton {
            Layout.fillWidth: true
            Kirigami.FormData.label: i18n("Devices to show:")
            text: i18nc("@option:radio Devices to show for scanning", "Scanners only")
            ButtonGroup.group: allDevicesGroup
            checked: !skanpage.configuration.showAllDevices
        }

        RadioButton {
            Layout.fillWidth: true
            text: i18nc("@option:radio Devices to show for scanning", "Scanners, cameras, and virtual devices")
            ButtonGroup.group: allDevicesGroup
            checked: skanpage.configuration.showAllDevices
            onCheckedChanged: skanpage.configuration.showAllDevices = checked
        }

        ComboBox {
            Kirigami.FormData.label: i18n("Default file format:")

            model: skanpage.formatModel
            textRole: "comment"
            valueRole: "nameFilter"

            onActivated: skanpage.configuration.defaultNameFilter = currentValue
            Component.onCompleted: {
                const index = indexOfValue(skanpage.configuration.defaultNameFilter)
                currentIndex = index >= 0 ? index : 0
            }
        }


        RowLayout {
            Kirigami.FormData.label: i18n("Default save location:")

            Kirigami.ActionTextField {
                id: folderLabel
                text: skanpage.configuration.defaultFolder
                readOnly: true
            }

            Button {
                icon.name: "document-open-folder"
                onClicked: selectFolderDialog.open()
            }
        }

    }

    RowLayout {
        id: buttonRow

        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: Kirigami.Units.smallSpacing
        }

        Button {
            action: closeAction
        }
    }

    Action {
        id: closeAction
        icon.name: "dialog-close"
        text: i18n("Close")
        shortcut: "Esc"
        onTriggered: settingsWindow.close()
    }

    @FILEDIALOG_MODE_LOAD_FOLDER@ {
        id: selectFolderDialog

        @FILEDIALOG_CURRENT_FOLDER@: skanpage.configuration.defaultFolder
        @FILEDIALOG_MODE_LOAD_FOLDER_SELECTEXISTING@
        @FILEDIALOG_MODE_LOAD_FOLDER_SELECTFOLDER@
        onAccepted: skanpage.configuration.defaultFolder = @FILEDIALOG_SELECTED_FILE@
    }
}
