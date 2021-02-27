/* ============================================================
 *
 * Copyright (C) 2015 by Kåre Särs <kare.sars@iki .fi>
 * Copyright (C) 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * ============================================================ */
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
    }
    
    Connections {
        target: Qt.application
        
        function onAboutToQuit() {
            persistentSettings.x = mainWindow.x;
            persistentSettings.y = mainWindow.y;
            persistentSettings.width = mainWindow.width;
            persistentSettings.height = mainWindow.height;
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
        icon.name: "scanner"
        text: i18n("Scan")
        shortcut: "SPACE"
        enabled: skanPage.progress === 100 && skanPage.openedDevice
        onTriggered: skanPage.startScan()
    }

    Action {
        id: scanUIAction
        icon.name: "configure"
        text: i18n("Scanner options")
        shortcut: "CTRL+SPACE"
        enabled: skanPage.openedDevice
        onTriggered: skanPage.showScannerUI();
    }
    
    Action {
        id: openMenuAction
        icon.name: "application-menu"
        text: i18n("Application menu")
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
            action: scanAction
        }
        
        MenuItem {
            action: saveDocAction
        }
        
        MenuItem { 
            action: newDocAction 
        }
               
        MenuItem {
            action: scanUIAction 
        } 
         
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
                
                ComboBox {
                    id: resCombo
                    textRole: "name"
                    valueRole: "resolution"
                    model: [ 
                        { name: i18n("Draft (75 DPI)"), resolution: 75 },
                        { name: i18n("Normal (150 DPI)"), resolution: 150 },
                        { name: i18n("High Quality (300 DPI)"), resolution: 300 },
                        { name: i18n("Best Quality (600 DPI)"), resolution: 600 }
                    ]
                    onCurrentValueChanged: {
                        skanPage.scanDPI = resCombo.currentValue
                    }
                    enabled: skanPage.progress === 100 && skanPage.openedDevice
                    currentIndex: 0
                    
                    Connections {
                        target: skanPage
                        function onScanDPIChanged() {
                            var dpiIndex = resCombo.indexOfValue(skanPage.scanDPI);
                            if (dpiIndex >= 0) {
                                resCombo.currentIndex = dpiIndex;
                            } else {
                                resCombo.currentIndex = 0;
                            }
                        }
                    }
                }

                ComboBox {
                    id: sizeCombo
                    implicitWidth: resCombo.implicitWidth / 2
                    model: skanPage.scanSizes
                    enabled: skanPage.progress === 100 && skanPage.openedDevice
                    onCurrentIndexChanged: {
                        skanPage.scanSizeIndex = currentIndex;
                    }
                    Connections {
                        target: skanPage
                        function onScanSizeIndexChanged() {
                            if (sizeCombo.currentIndex != skanPage.scanSizeIndex) {
                                sizeCombo.currentIndex = skanPage.scanSizeIndex;
                            }
                        }
                    }

                }
                
                ComboBox {
                    id: modeCombo
                    implicitWidth: resCombo.implicitWidth / 2

                    textRole: "name"
                    valueRole: "selection"
                    model: [ 
                        { name: i18n("Color"), selection: true },
                        { name: i18n("Gray"), selection: false }
                        ]
                    enabled: skanPage.progress === 100 && skanPage.openedDevice
                    currentIndex: 0
                    
                    onCurrentValueChanged:  {
                        skanPage.colorMode = modeCombo.currentValue;
                    }
                    
                    Connections {
                        target: skanPage
                        function onColorModeChanged() {
                            if (skanPage.colorMode) {
                                modeCombo.currentIndex = 0;
                            } else {
                                modeCombo.currentIndex = 1;
                            }
                        }
                    }
                }
                                
                ToolButton { 
                    action: scanUIAction
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
        }
        
        DevicesView {
            id: devicesView
            
            visible: !skanPage.openedDevice
            
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
        }
    }
    
    FileDialog {
        id: saveFileDialog
        folder: shortcuts.documents
        selectExisting: false
        selectMultiple: false
        nameFilters: [ "PDF files (*.pdf)", "JPEG files (*.jpg)", "PNG files (*.png)", "All files (*)" ]
        onAccepted: {
            skanPage.documentModel.save(fileUrl)
        }
    }
}
