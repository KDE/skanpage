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

import org.kde.kirigami 2.5 as Kirigami
import org.kde.skanpage 1.0

Window {
    id: settingsWindow

    title: i18n("Skanpage Settings")
    color: Kirigami.Theme.backgroundColor

    flags: Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint
        | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint

    minimumWidth: 500
    minimumHeight: 300

    Kirigami.FormLayout {
        anchors.fill:parent

        CheckBox {
            Layout.fillWidth: true

            text: i18n("Show all devices including virtual and cameras")
            checked: skanpage.configuration.showAllDevices
            onClicked: skanpage.configuration.showAllDevices = checked
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: i18n("Default file format:")
            }

            ComboBox {
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
        }

        ColumnLayout {
            Layout.fillWidth: true

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: i18n("Default save location:")
                }

                Button {
                    icon.name: "document-save"
                    onClicked: selectFolderDialog.open()
                }
            }

            TextField {
                id: folderLabel
                readOnly: true
                Layout.fillWidth: true

                text: skanpage.configuration.defaultFolder
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
