/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami

import org.kde.skanpage

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

            onActivated: function (index) {
                skanpage.configuration.defaultNameFilterIndex = index
            }
            Component.onCompleted: {
                const index = skanpage.configuration.defaultNameFilterIndex
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

    FolderDialog {
        id: selectFolderDialog

        currentFolder: skanpage.configuration.defaultFolder
        onAccepted: skanpage.configuration.defaultFolder = selectedFolder
    }
}
