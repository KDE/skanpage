/**
 * SPDX-FileCopyrightText: 2022 by Alexander Stippich <a.stippich@gmx.net>
 * SPDX-FileCopyrightText: 2024 by Milena Cole <mkoul@mail.ru>
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

    Kirigami.ScrollablePage {
        anchors.fill: parent

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
                id: formatComboBox

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

                Layout.fillWidth: true
                Kirigami.ActionTextField {
                    id: folderLabel
                    Layout.fillWidth: true
                    text: skanpage.configuration.defaultFolder
                    readOnly: true
                }

                Button {
                    icon.name: "document-open-folder"
                    onClicked: selectFolderDialog.open()
                }
            }

            Kirigami.ActionTextField {
                Kirigami.FormData.label: i18n("Title template:")

                id: templateLabel

                Layout.fillWidth: true
                text: skanpage.configuration.nameTemplate
                onTextEdited: skanpage.configuration.nameTemplate = templateLabel.text
            }

            Label {
                text: i18n("Preview:")
            }

            Label {
                id: namePreview

                Layout.fillWidth: true
                wrapMode: Text.WrapAnywhere
                text: skanpage.nameTemplate.parseSamples(folderLabel.text, templateLabel.text, skanpage.formatModel.getData(formatComboBox.currentIndex, FormatModel.SuffixRole))
            }

            Text {
                id: showSamples
                text: i18n("Show Samples")
                color: "steelblue"
                property bool isSamplesVisable: false

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        showSamples.isSamplesVisable = ! showSamples.isSamplesVisable
                        if (showSamples.isSamplesVisable) {showSamples.text = i18n("Hide Samples"); samplesFrame.visible = true}
                        else {showSamples.text = i18n("Show Samples"); samplesFrame.visible = false}
                    }
                }
            }

            Frame {
                id: samplesFrame

                visible: false

                GridLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    columns: 2
                    rowSpacing: Kirigami.Units.smallSpacing
                    columnSpacing: Kirigami.Units.smallSpacing

                    Button {
                        text: skanpage.nameTemplate.getSample(0)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(0)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(1)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(1)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(2)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(2)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(3)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(3)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(4)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(4)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(5)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(5)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(6)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(6)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(7)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(7)
                    }

                    Button {
                        text: skanpage.nameTemplate.getSample(8)
                        Layout.fillWidth: true
                        onClicked: addSample(text)
                    }

                    Label {
                        text: skanpage.nameTemplate.getSampleName(8)
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
        }
    }

    function addSample(text) {
        const pos = templateLabel.cursorPosition;
        templateLabel.text = skanpage.nameTemplate.addSampleToText(templateLabel.text, text, pos);
        skanpage.configuration.nameTemplate = templateLabel.text;
        templateLabel.focus = true;
        templateLabel.cursorPosition = pos + text.length
    }

    FolderDialog {
        id: selectFolderDialog

        currentFolder: skanpage.configuration.defaultFolder
        onAccepted: {
            skanpage.configuration.defaultFolder = selectedFolder
        }
    }
}
