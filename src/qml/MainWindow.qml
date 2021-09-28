/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.0

import org.kde.kirigami 2.5 as Kirigami
import org.kde.skanpage 1.0

ApplicationWindow {
    id: mainWindow

    visible: true

    title: skanpage.deviceVendor && skanpage.deviceModel ?
        i18nc("document title - scanner device - app title", "%1 ― %2 %3 ― Skanpage",
        mainDocument.name, skanpage.deviceVendor, skanpage.deviceModel)
        : i18nc("document title: app title", "%1 ― Skanpage", mainDocument.name)

    width: persistentSettings.width
    height: persistentSettings.height
    x: persistentSettings.x
    y: persistentSettings.y

    minimumWidth: mainToolBar.implicitWidth
    minimumHeight: 400

    Settings {
        id: persistentSettings

        property int x: 0
        property int y: 0
        property int width: 950
        property int height: 550
        property int optionWidth: 600
        property int optionHeight: 400
        property var splitViewState
    }

    Connections {
        target: Qt.application

        function onAboutToQuit() {
            persistentSettings.x = mainWindow.x
            persistentSettings.y = mainWindow.y
            persistentSettings.width = mainWindow.width
            persistentSettings.height = mainWindow.height
            persistentSettings.splitViewState = mainDocument.splitView.saveState()
            persistentSettings.optionHeight = optionsWindow.height
            persistentSettings.optionWidth = optionsWindow.width
        }
    }

    Connections {
        target: skanpage

        function onNewUserMessage(level, message) {
            errorMessage.text = message
            if (level == Skanpage.ErrorMessage) {
                errorMessage.type = Kirigami.MessageType.Error
            } else {
                errorMessage.type = Kirigami.MessageType.Information
            }
            labelWidth.text = message
            errorMessage.visible = true
            hideNotificationTimer.start()
        }
    }

    Action {
        id: newDocAction
        icon.name: "document-replace"
        text: i18n("Clear")
        shortcut: StandardKey.New
        enabled: skanpage.documentModel.count !== 0
        onTriggered: skanpage.documentModel.clearData()
    }

    Action {
        id: saveDocAction
        icon.name: "document-save"
        text: i18n("Save")
        shortcut: StandardKey.Save
        enabled: skanpage.documentModel.count !== 0
        onTriggered: saveFileDialog.open()
    }

    Action {
        id: quitAction
        icon.name: "window-close"
        text: i18n("Quit")
        shortcut: StandardKey.Quit
        onTriggered: Qt.quit()
    }

    Action {
        id: scanAction
        icon.name: "document-scan"
        text: i18n("Scan")
        shortcut: "SPACE"
        enabled: skanpage.applicationState == Skanpage.ReadyForScan
        onTriggered: skanpage.startScan()
    }

    Action {
        id: cancelAction
        icon.name: "dialog-cancel"
        text: i18n("Cancel")
        shortcut: "Esc"
        enabled: skanpage.applicationState == Skanpage.ScanInProgress
        onTriggered: skanpage.cancelScan()
    }

    Action {
        id: optionsAction
        icon.name: "configure"
        text: i18n("Scanner options")
        shortcut: "CTRL+SPACE"
        enabled: skanpage.applicationState == Skanpage.ReadyForScan
        onTriggered: optionsWindow.show()
    }

    Action {
        id: printAction
        icon.name: "document-print"
        text: i18n("Print")
        shortcut: StandardKey.Print
        enabled: skanpage.documentModel.count !== 0
        onTriggered: skanpage.documentModel.print()
    }

    Action {
        id: openMenuAction
        icon.name: "application-menu"
        onTriggered: {
            if (applicationMenu.visible) {
                applicationMenu.close()
            } else {
                applicationMenu.popup(mainWindow.width - applicationMenu.width, mainToolBar.height)
            }
        }
    }

    Action {
        id: showAboutAction
        icon.name: "skanpage"
        text: i18n("About Skanpage")
        onTriggered: skanpage.showAboutDialog()
    }

    Action {
        id: reselectDevicesAction
        icon.name: "view-refresh"
        text: i18n("Reselect scanning device")
        onTriggered: skanpage.reloadDevicesList()
        enabled: skanpage.applicationState == Skanpage.ReadyForScan
    }

    Menu {
        id: applicationMenu

        MenuItem {
            action: reselectDevicesAction
        }

        MenuItem {
            action: showAboutAction
        }

        MenuItem {
            action: quitAction
        }
    }

    Kirigami.InlineMessage {
        id: errorMessage
        width: labelWidth.width + Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
        height: Math.max(labelWidth.height, Kirigami.Units.iconSizes.medium) + Kirigami.Units.largeSpacing
        z: 2

        anchors {
            top: parent.top
            topMargin: mainToolBar.height + Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        Timer {
            id: hideNotificationTimer
            interval: 5000
            onTriggered: errorMessage.visible = false
        }

        TextMetrics {
            id: labelWidth
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        spacing: 0

        ToolBar {
            id: mainToolBar
            Layout.fillWidth: true

            RowLayout {
                id: toolbarRow
                anchors.fill: parent
                spacing: 0

                Item {

                    implicitWidth: Math.max(children[0].width, children[1].width)
                    implicitHeight: Math.max(children[0].height, children[1].height)

                    ToolButton {
                        anchors.right: parent.right
                        action: scanAction
                        visible: !skanpage.applicationState == Skanpage.ScanInProgress
                    }

                    ToolButton {
                        anchors.right: parent.right
                        action: cancelAction
                        visible: skanpage.applicationState == Skanpage.ScanInProgress
                    }
                }

                ToolButton {
                    action: saveDocAction
                }

                ToolButton {
                    action: newDocAction
                }

                OptionDelegate {
                    modelItem: skanpage.resolutionOption
                    onValueChanged: skanpage.resolutionOption.value = value
                    enabled: skanpage.applicationState == Skanpage.ReadyForScan
                }

                OptionDelegate {
                    modelItem: skanpage.pageSizeOption
                    onValueChanged: skanpage.pageSizeOption.value = value
                    enabled: skanpage.applicationState == Skanpage.ReadyForScan
                }

                OptionDelegate {
                    modelItem: skanpage.sourceOption
                    onValueChanged: skanpage.sourceOption.value = value
                    enabled: skanpage.applicationState == Skanpage.ReadyForScan
                }

                OptionDelegate {
                    modelItem: skanpage.scanModeOption
                    onValueChanged: skanpage.scanModeOption.value = value
                    enabled: skanpage.applicationState == Skanpage.ReadyForScan
                }

                ToolButton {
                    action: optionsAction
                    visible: skanpage.optionsModel.rowCount > 0
                }

                Item {
                    id: toolbarSpacer
                    Layout.fillWidth: true
                }

                ToolButton {
                    action: printAction
                }

                ToolButton {
                    action: openMenuAction
                }
            }
        }

        DocumentView {
            id: mainDocument

            visible: skanpage.applicationState == Skanpage.ReadyForScan || skanpage.applicationState == Skanpage.ScanInProgress

            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true

            Component.onCompleted: {
                mainDocument.splitView.restoreState(persistentSettings.splitViewState)
            }
        }

        DevicesView {
            id: devicesView

            visible: skanpage.applicationState != Skanpage.ReadyForScan && skanpage.applicationState != Skanpage.ScanInProgress 

            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
        }
    }

    OptionsWindow {
        id: optionsWindow

        height: persistentSettings.optionHeight
        width: persistentSettings.optionWidth
    }

    FileDialog {
        id: saveFileDialog
        folder: shortcuts.documents
        selectExisting: false
        selectMultiple: false
        nameFilters: [ i18n("PDF files (*.pdf)"), i18n("JPEG files (*.jpg)"), i18n("PNG files (*.png)"), i18n("All files (*)") ]
        onAccepted: skanpage.documentModel.save(fileUrl)
    }
    
    GlobalMenu {
        
    }
}
