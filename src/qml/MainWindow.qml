/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.3

import org.kde.kirigami 2.19 as Kirigami
import org.kde.skanpage 1.0

ApplicationWindow {
    id: mainWindow

    visible: true

    title: i18nc("document title: app title", "%1 ― Skanpage", mainView.name)
    color: Kirigami.Theme.backgroundColor

    width: skanpage.stateConfiguration.width
    height: skanpage.stateConfiguration.height
    x: skanpage.stateConfiguration.x
    y: skanpage.stateConfiguration.y

    minimumWidth: mainToolBar.implicitWidth
    minimumHeight: 400

    Connections {
        target: Qt.application

        function onAboutToQuit() {
            skanpage.stateConfiguration.x = mainWindow.x
            skanpage.stateConfiguration.y = mainWindow.y
            skanpage.stateConfiguration.width = mainWindow.width
            skanpage.stateConfiguration.height = mainWindow.height
            skanpage.stateConfiguration.splitViewItemWidth = mainView.splitViewItemWidth
            skanpage.stateConfiguration.showOptions = mainView.showOptions
            skanpage.stateConfiguration.showAllOptions = mainView.optionsPanel.allOptionsAction.checked
            skanpage.stateConfiguration.shareHeight = shareWindow.height
            skanpage.stateConfiguration.shareWidth = shareWindow.width
            skanpage.stateConfiguration.exportHeight = exportWindow.height
            skanpage.stateConfiguration.exportWidth = exportWindow.width
            skanpage.stateConfiguration.settingsHeight = settingsWindow.height
            skanpage.stateConfiguration.settingsWidth = settingsWindow.width
        }
    }

    Connections {
        target: skanpage

        function onNewUserMessage(level, message) {
            errorMessage.text = message
            if (level == SkanpageUtils.ErrorMessage) {
                errorMessage.type = Kirigami.MessageType.Error
            } else {
                errorMessage.type = Kirigami.MessageType.Information
            }
            labelWidth.text = message
            errorMessage.visible = true
            hideNotificationTimer.start()
        }

        function onApplicationStateChanged(state) {
            if (state == Skanpage.SearchingForDevices) {
                stackView.push(devicesLoading)
            } else if (state == Skanpage.DeviceSelection) {
                stackView.replace(deviceSelection)
            } else if (state == Skanpage.ReadyForScan) {
                while(stackView.depth > 1) {
                    stackView.pop()
                }
            }
        }
    }

    ShortcutsAction {
        id: newDocAction
        icon.name: "edit-delete-remove"
        text: i18n("Discard All")
        shortcutsName: "New"
        enabled: skanpage.documentModel.count !== 0
        onTriggered: skanpage.documentModel.clearData()
    }

    ShortcutsAction {
        id: saveDocAction
        icon.name: "document-save"
        text: i18n("Save All")
        shortcutsName: "Save"
        enabled: skanpage.documentModel.count !== 0
        onTriggered: saveFileDialog.open()
    }
    
    ShortcutsAction {
        id: exportDocAction
        icon.name: "document-save"
        text: i18n("Export PDF")
        enabled: skanpage.documentModel.count !== 0
        onTriggered: exportWindow.show()
    }

    ShortcutsAction {
        id: previewAction
        icon.name: "document-preview"
        text: i18n("Preview")
        shortcut: "P"
        enabled: skanpage.applicationState == Skanpage.ReadyForScan
        onTriggered: skanpage.preview()
    }

    ShortcutsAction {
        id: scanAction
        icon.name: "document-scan"
        text: i18n("Scan")
        shortcut: "SPACE"
        enabled: skanpage.applicationState == Skanpage.ReadyForScan
        onTriggered: {
            skanpage.scanArea = mainView.activeDocument.getScanArea()
            skanpage.startScan()
        }
    }

    Action {
        id: cancelAction
        icon.name: "dialog-cancel"
        text: i18n("Cancel")
        shortcut: "Esc"
        enabled: skanpage.applicationState == Skanpage.ScanInProgress
        onTriggered: skanpage.cancelScan()
    }

    ShortcutsAction {
        id: showOptionsAction
        icon.name: "configure"
        text: i18n("Show Scanner Options")
        shortcut: "CTRL+O"
        checkable: true
        checked: mainView.showOptions
        onTriggered: mainView.showOptions = !mainView.showOptions
    }

    ShortcutsAction {
        id: shareAction
        icon.name: "document-share"
        text: i18n("Share")
        enabled: skanpage.documentModel.count !== 0
        onTriggered: shareWindow.show()
    }

    ShortcutsAction {
        id: printAction
        icon.name: "document-print"
        text: i18n("Print")
        shortcutsName: "Print"
        enabled: skanpage.documentModel.count !== 0
        onTriggered: skanpage.print()
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

    ShortcutsAction {
        id: showAboutAction
        icon.name: "skanpage"
        text: i18n("About Skanpage")
        onTriggered: aboutWindow.show()
    }

    ShortcutsAction {
        id: settingsAction
        icon.name: "settings-configure"
        text: i18n("Configure Skanpage…")
        shortcutsName: "Preferences"
        onTriggered: settingsWindow.show()
    }

    ShortcutsAction {
        id: shortcutSettingsAction
        icon.name: "configure-shortcuts"
        text: i18n("Configure Keyboard Shortcuts…")
        shortcutsName: "KeyBindings"
        onTriggered: skanpage.showShortcutsDialog()
    }
    
    ShortcutsAction {
        id: quitAction
        icon.name: "window-close"
        text: i18n("Quit")
        shortcutsName: "Quit"
        onTriggered: Qt.quit()
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

                    implicitWidth: Math.max(children[0].implicitWidth, children[1].implicitWidth)
                    implicitHeight: Math.max(children[0].implicitHeight, children[1].implicitHeight)

                    Row {
                        visible: skanpage.applicationState !== Skanpage.ScanInProgress

                        ToolButton {
                            action: previewAction
                        }

                        ToolButton {
                            action: scanAction
                        }
                    }

                    ToolButton {
                        anchors.fill: parent
                        action: cancelAction
                        visible: skanpage.applicationState === Skanpage.ScanInProgress
                    }
                }

                ToolButton {
                    action: saveDocAction
                }
      
                ToolButton {
                    action: exportDocAction 
                }

                ToolButton {
                    action: newDocAction
                }

                ToolButton {
                    action: shareAction
                }

                ToolButton {
                    action: printAction
                }

                Item {
                    id: toolbarSpacer
                    Layout.fillWidth: true
                }

                ToolButton {
                    action: showOptionsAction
                    visible: skanpage.applicationState === Skanpage.ReadyForScan || skanpage.applicationState === Skanpage.ScanInProgress
                }

                ToolButton {
                    action: openMenuAction
                }
            }
        }

        Menu {
            id: applicationMenu

            MenuItem {
                action: settingsAction
            }

            MenuItem {
                action: shortcutSettingsAction
            }

            MenuItem {
                action: showAboutAction
            }
        }

        StackView {
            id: stackView
            
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            initialItem: ContentView {
                id: mainView

                showOptions: skanpage.stateConfiguration.showOptions
                splitViewPreferredWidth: skanpage.stateConfiguration.splitViewItemWidth
                focus: true

                onSaveSinglePage: {
                    saveFileDialog.pageNumbers.push(pageNumber)
                    saveFileDialog.open()
                }
            }
        }
    }

    Component {
        id: deviceSelection

        DeviceSelection {
            focus: true
        }
    }

    Component {
        id: devicesLoading

        DevicesLoading {

        }
    }

    ShareWindow {
        id: shareWindow

        height: skanpage.stateConfiguration.shareHeight
        width: skanpage.stateConfiguration.shareWidth

        onError: {
            errorMessage.text = errorText
            errorMessage.type = Kirigami.MessageType.Error
            labelWidth.text = message
            errorMessage.visible = true
            hideNotificationTimer.start()
        }
    }

    Window {
        id: aboutWindow

        title: i18n("About Skanpage")
        color: Kirigami.Theme.backgroundColor

        flags: Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint
            | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint

        minimumWidth: 600
        minimumHeight: 500

        Kirigami.AboutItem {
            aboutData: _aboutData
            anchors.fill: parent
        }
    }

    FileDialog {
        id: saveFileDialog

        property var pageNumbers: []

        folder: skanpage.configuration.defaultFolder
        selectExisting: false
        selectMultiple: false
        nameFilters: skanpage.formatModel.formatFilter()
        selectedNameFilter: skanpage.configuration.defaultNameFilter
        onAccepted: {
            skanpage.documentModel.save(fileUrl, pageNumbers)
            pageNumbers = []
        }
        onRejected: pageNumbers = []
    }

    GlobalMenu {
       newDocAction: newDocAction
       saveDocAction: saveDocAction
       previewAction: previewAction
       scanAction: scanAction
       cancelAction: cancelAction
       allOptionsAction: mainView.optionsPanel.allOptionsAction
       showOptionsAction: showOptionsAction
       printAction: printAction
       showAboutAction: showAboutAction
       reselectDevicesAction: mainView.optionsPanel.reselectDevicesAction
       quitAction: quitAction
       shareAction: shareAction
    }    

    ExportWindow {
        id: exportWindow

        height: skanpage.stateConfiguration.exportHeight
        width: skanpage.stateConfiguration.exportWidth
    }

    SettingsWindow {
        id: settingsWindow
        height: skanpage.stateConfiguration.settingsHeight
        width: skanpage.stateConfiguration.settingsWidth
    }

    Component.onCompleted: {
        skanpage.optionsModel.showAllOptions(skanpage.stateConfiguration.showAllOptions)
        if (skanpage.applicationState == Skanpage.SearchingForDevices) {
            stackView.push(devicesLoading)
        } else if (skanpage.applicationState == Skanpage.DeviceSelection) {
            stackView.push(deviceSelection)
        }
    }
}
