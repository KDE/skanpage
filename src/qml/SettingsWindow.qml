/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.3

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

        ButtonGroup { id: allDevicesGroup }

        RadioButton {
            Kirigami.FormData.label: i18n("Devices to show:")
            text: i18nc("@option:radio Devices to show for scanning", "Scanners only")
            ButtonGroup.group: allDevicesGroup
            checked: !skanpage.configuration.showAllDevices
        }

        RadioButton {
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
                var index = indexOfValue(skanpage.configuration.defaultNameFilter)
                if (index < 0) {
                    currentIndex = 0
                } else {
                    currentIndex = index
                }
            }
        }


        RowLayout {
            Kirigami.FormData.label: i18n("Default save location:")

            Kirigami.ActionTextField {
                id: folderLabel
                text: skanpage.configuration.defaultFolder

                rightActions: Kirigami.Action {
                    icon.name: "edit-clear"
                    visible: folderLabel.text !== ""
                    onTriggered: {
                        folderLabel.clear();
                    }
                }

                onEditingFinished: skanpage.configuration.defaultFolder = text
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

    FileDialog {
        id: selectFolderDialog

        folder: skanpage.configuration.defaultFolder
        selectExisting: true
        selectMultiple: false
        selectFolder: true
        onAccepted: skanpage.configuration.defaultFolder = fileUrl
    }
}
