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

    Action {
        id: newDocAction
        icon.name: "document-replace"
        text: i18n("Clear")
        shortcut: StandardKey.New
        enabled: skanPage.documentModel.count !== 0
        onTriggered: skanPage.documentModel.clearData()
    }

    Action {
        id: saveDocAction
        icon.name: "document-save"
        text: i18n("Save")
        shortcut: StandardKey.Save
        enabled: skanPage.documentModel.count !== 0
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
        enabled: skanPage.progress === 100 && skanPage.openedDevice
        onTriggered: skanPage.startScan()
    }

    Action {
        id: optionsAction
        icon.name: "configure"
        text: skanPage.deviceVendor && skanPage.deviceModel ? i18nc("Device vendor and device model name", "Options for %1 %2", skanPage.deviceVendor, skanPage.deviceModel) : i18n("Scanner options")
        shortcut: "CTRL+SPACE"
        enabled: skanPage.optionsModel.rowCount > 0
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
        onTriggered: skanPage.showAboutDialog();
    }
    
    Action {
        id: reselectDevicesAction
        icon.name: "view-refresh"
        text: i18n("Reselect scanning device")
        onTriggered: skanPage.reloadDevicesList();
        enabled: skanPage.progress === 100
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
                    modelItem: skanPage.resolutionOption
                    onValueChanged: skanPage.resolutionOption.value = value
                }
                
                OptionDelegate {
                    modelItem: skanPage.pageSizeOption
                    onValueChanged: skanPage.pageSizeOption.value = value
                }  
                
                OptionDelegate {
                    modelItem: skanPage.sourceOption
                    onValueChanged: skanPage.sourceOption.value = value
                }
                
                OptionDelegate {
                    modelItem: skanPage.scanModeOption
                    onValueChanged: skanPage.scanModeOption.value = value
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
            
            visible: skanPage.openedDevice
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
            
            Component.onCompleted: {
                mainDocument.splitView.restoreState(persistentSettings.splitViewState)
            }
        }
        
        DevicesView {
            id: devicesView
            
            visible: !skanPage.openedDevice
            
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
            skanPage.documentModel.save(fileUrl)
        }
    }
}
