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
import QtQuick.Controls 1.3 as QQC1
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: mainWindow
    
    title: qsTr("%1: Skanpage").arg(docName)
    property string docName: tabView.count > 0 ? tabView.getTab(tabView.currentIndex).title : ""
    width: 800
    height: 550
    visible: true

    minimumWidth: toolbarRow.childrenRect.width - toolbarSpacer.width
    minimumHeight: 400

    Action {
        id: newDocAction
        icon.name: "document-new"
        text: qsTr("New Document")
        shortcut: StandardKey.New
        onTriggered: addTab();
    }

    Action {
        id: saveDocAction
        icon.name: "document-save"
        text: qsTr("Save")
        shortcut: StandardKey.Save
        onTriggered: {
            if (tabView.getTab(tabView.currentIndex).item) {
                tabView.getTab(tabView.currentIndex).item.save();
            }
        }
    }

    Action {
        id: closeDocAction
        icon.name: "document-close"
        text: qsTr("Close")
        shortcut: StandardKey.Close
        onTriggered: tabView.removeTab(tabView.currentIndex); // FIXME ask if modified
    }

    Action {
        id: quitAction
        icon.name: "window-close"
        text: qsTr("Quit")
        shortcut: StandardKey.Quit
        onTriggered: Qt.quit();
    }

    Action {
        id: scanAction
        icon.name: "scanner"
        text: qsTr("Scan")
        shortcut: "SPACE"
        enabled: skanPage.progress === 100
        onTriggered: skanPage.startScan();
    }

    Action {
        id: scanUIAction
        icon.name: "configure"
        text: qsTr("Scanner options")
        shortcut: "CTRL+SPACE"
        onTriggered: skanPage.showScannerUI();
    }
    
    Action {
        id: openMenuAction
        icon.name: "application-menu"
        text: qsTr("Application menu")
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
            action: newDocAction 
        }
        
        MenuItem {
            action: scanAction
        }
        
        MenuItem {
            action: saveDocAction
        }
        
        MenuItem {
            action: closeDocAction 
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
        
        ToolBar {
            id: mainToolBar
            Layout.fillWidth: true
            
            RowLayout {
                id: toolbarRow
                anchors.fill: parent

                ToolButton {
                    action: saveDocAction 
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
                        var dpi = skanPage.scanDPI();
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
                            skanPage.setScanDPI(resolutions.get(currentIndex).resolution);
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
                
                ToolButton { 
                    action: scanAction
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

        QQC1.TabView {
            id: tabView
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
            frameVisible: false
            tabsVisible: count > 1

            onCurrentIndexChanged: {
                if (tabView.getTab(tabView.currentIndex).item) {
                    tabView.getTab(tabView.currentIndex).item.grabScanner();
                    //tabView.getTab(tabView.currentIndex).item.forceActiveFocus();
                }
            }
            onCountChanged: {
                if (count > 0) {
                    if (tabView.getTab(tabView.currentIndex).item) {
                        tabView.getTab(tabView.currentIndex).item.grabScanner();
                        tabView.getTab(tabView.currentIndex).item.forceActiveFocus();
                    }
                }
            }
            Keys.onPressed: {
                //console.log("tabView", event)
                tabView.getTab(tabView.currentIndex).item.forceActiveFocus();
            }
        }
    }
    
    ListModel {
        id: resolutions
        ListElement { name: qsTr("Draft"); resolution: 75 }
        ListElement { name: qsTr("Normal"); resolution: 150 }
        ListElement { name: qsTr("High Quality"); resolution: 300 }
        ListElement { name: qsTr("Best Quality"); resolution: 600 }
    }
    
    function addTab() {
        tabView.addTab("foo", docComponent);
        tabView.currentIndex = tabView.count-1;
        var tab = tabView.getTab(tabView.count-1);
        tab.title = Qt.binding(function() {return tab.item.name});
        tab.item.grabScanner();
        tab.item.forceActiveFocus();
    }

    Component.onCompleted: {
        addTab();
    }

    Component {
        id: docComponent
        Document {
            anchors.fill: parent
        }
    }

}
