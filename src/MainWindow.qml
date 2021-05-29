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
        
    title: i18nc("document title: app title", "%1 ― Skanpage", mainDocument.name)
    
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
        enabled: skanpage.progress === 100 && skanpage.openedDevice
        onTriggered: skanpage.startScan()
    }

    Action {
        id: optionsAction
        icon.name: "configure"
        text: skanpage.deviceVendor && skanpage.deviceModel ? i18nc("Device vendor and device model name", "Options for %1 %2", skanpage.deviceVendor, skanpage.deviceModel) : i18n("Scanner options")
        shortcut: "CTRL+SPACE"
        enabled: skanpage.optionsModel.rowCount > 0
        onTriggered: optionsWindow.show();
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
        icon.name: "scanner"
        text: i18n("About Skanpage")
        onTriggered: skanpage.showAboutDialog();
    }
    
    Action {
        id: reselectDevicesAction
        icon.name: "view-refresh"
        text: i18n("Reselect scanning device")
        onTriggered: skanpage.reloadDevicesList();
        enabled: skanpage.progress === 100
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
                
                ToolButton { 
                    action: scanAction
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
                }
                
                OptionDelegate {
                    modelItem: skanpage.pageSizeOption
                    onValueChanged: skanpage.pageSizeOption.value = value
                }  
                
                OptionDelegate {
                    modelItem: skanpage.sourceOption
                    onValueChanged: skanpage.sourceOption.value = value
                }
                
                OptionDelegate {
                    modelItem: skanpage.scanModeOption
                    onValueChanged: skanpage.scanModeOption.value = value
                }
             
                ToolButton { 
                    action: optionsAction
                }

                Item {
                    id: toolbarSpacer
                    Layout.fillWidth: true
                }

                ToolButton { 
                    action: openMenuAction
                }
            }
        }
  
        DocumentView {
            id: mainDocument
            
            visible: skanpage.applicationState == Skanpage.ReadyForScan

            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
            
            Component.onCompleted: {
                mainDocument.splitView.restoreState(persistentSettings.splitViewState)
            }
        }
        
        DevicesView {
            id: devicesView
            
            visible: skanpage.applicationState != Skanpage.ReadyForScan
            
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
        }
    }
    
    OptionsWindow {
        id: optionsWindow
    }
    
    FileDialog {
        id: saveFileDialog
        folder: shortcuts.documents
        selectExisting: false
        selectMultiple: false
        nameFilters: [ i18n("PDF files (*.pdf)"), i18n("JPEG files (*.jpg)"), i18n("PNG files (*.png)"), i18n("All files (*)") ]
        onAccepted: {
            skanpage.documentModel.save(fileUrl)
        }
    }
}
