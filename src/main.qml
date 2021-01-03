/* ============================================================
 *
 * Copyright (C) 2015 by Kåre Särs <kare.sars@iki .fi>
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
        
    title: i18nc("document title: app title", "%1: Skanpage", docName)
    property string docName: mainDocument.name ? mainDocument.name  : ""
    
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
        onTriggered: skanPage.documentModel.clearData()
    }

    Action {
        id: saveDocAction
        icon.name: "document-save"
        text: i18n("Save")
        shortcut: StandardKey.Save
        //onTriggered: saveDialog.open(skanPage.documentModel.name)
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
        enabled: skanPage.progress === 100
        onTriggered: skanPage.startScan()
    }

    Action {
        id: scanUIAction
        icon.name: "configure"
        text: i18n("Scanner options")
        shortcut: "CTRL+SPACE"
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
            action: quitAction 
        }
    }
    
    ColumnLayout {
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
                
                ToolButton { 
                    action: scanUIAction
                }
                
                ComboBox {
                    id: resCombo
                    model: resolutions
                    textRole: "name"
                    enabled: skanPage.progress === 100
                    property bool complete: false
                    Component.onCompleted: {
                        var dpi = skanPage.scanDPI;
                        for (var i=0; i<resolutions.count; i++) {
                            if (resolutions.get(i).resolution >= dpi) {
                                currentIndex = i;
                                break;
                            }
                        }
                        complete = true;
                    }
                    onCurrentIndexChanged: {
                        if (complete) {
                            //console.log("setting DPI", resolutions.get(currentIndex).resolution)
                            skanPage.scanDPI = resolutions.get(currentIndex).resolution
                        }
                    }
                }

                ComboBox {
                    id: sizeCombo
                    model: skanPage.scanSizes
                    enabled: skanPage.progress === 100
                    onCurrentIndexChanged: {
                        skanPage.scanSizeIndex = currentIndex;
                    }
                    Connections {
                        target: skanPage
                        onScanSizeIndexChanged: {
                            if (sizeCombo.currentIndex != skanPage.scanSizeIndex) {
                                sizeCombo.currentIndex = skanPage.scanSizeIndex;
                            }
                        }
                    }
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
        
        Kirigami.InlineMessage
        {
            Layout.fillWidth: true 
            
            type: Kirigami.MessageType.Error
        }
        
        Document {
            id: mainDocument
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
        nameFilters: [ "Image and PDF files (*.jpg *.png *.pdf)", "All files (*)" ]
        onAccepted: {
            skanPage.documentModel.save(fileUrl)
        }
    }
    
    ListModel {
        id: resolutions
        Component.onCompleted: {
            resolutions.append({ name: i18n("Draft (75 DPI)"), resolution: 75 });
            resolutions.append({ name: i18n("Normal (150 DPI)"), resolution: 150 });
            resolutions.append({ name: i18n("High Quality (300 DPI)"), resolution: 300 });
            resolutions.append({ name: i18n("Best Quality (600 DPI)"), resolution: 600 });
        }
    }

}
