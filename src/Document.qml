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
import QtQuick.Controls 2.14 
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.skanpage 1.0

Item {
    id: doc

    focus: true
    clip: true

    readonly property string fname: pages.name === "" ? i18n("Unnamed") : pages.name

    readonly property string name: pages.changed ?  "* " + fname : fname;

    property var pages: skanPage.documentModel

    function addImage() {
        skanPage.startScan()
    }

    function save() {
        saveDialog.open(pages.name)
    }

    SaveDialog {
        id: saveDialog
        doc: pages
    }

    SystemPalette { id: palette; colorGroup: SystemPalette.Inactive }

    Timer {
        id: delTmr
        interval: 1
        property int delIndex
        onTriggered: {
            pages.removeImage(delIndex);
            if (listView.count === 0) {
                bigImage.source = "";
            }
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        ScrollView {
            id: scrollView

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            
            SplitView.fillHeight: true
            SplitView.preferredWidth: parent.width / 4         

            ListView {
                id: listView
                anchors.fill: parent

                displaced: Transition {
                    NumberAnimation { properties: "x,y"; easing.type: Easing.OutQuad }
                }

                model: pages

                onCurrentItemChanged: {
                    bigImage.source = listView.currentItem ? listView.currentItem.imageSrc : ""
                    bigImage.zoomScale = Math.min(imageViewer.availableWidth / bigImage.sourceSize.width, 1)
                }

                delegate: Item {
                    id: delegateRoot
                    focus: index === listView.currentIndex

                    property string imageSrc: "file://"+model.name
                    property int index: model.index
                    width: listView.width;
                    height: icon.height+2*Screen.pixelDensity

                    MouseArea {
                        id:mouseArea
                        anchors.fill: parent
                        drag.target: icon

                        acceptedButtons: Qt.LeftButton | Qt.RightButton


                        onClicked: {
                            listView.currentIndex = index;
                            if (mouse.button === Qt.RightButton) {
                                myContextMenu.popup();
                            }
                        }
                    }

                    DropArea {
                        anchors.fill: parent
                        onEntered: {
                            pages.moveImage(drag.source.index, delegateRoot.index, 1);
                        }
                    }

                    Item {
                        id: icon
                        width: iconImage.width
                        height: iconImage.height + number.height + Screen.pixelDensity*0.5
                        anchors {
                            horizontalCenter: parent.horizontalCenter;
                            verticalCenter: parent.verticalCenter
                        }

                        Drag.active: mouseArea.drag.active
                        Drag.source: delegateRoot
                        Drag.hotSpot.x: width / 2
                        Drag.hotSpot.y: height / 2

                        states: [
                        State {
                            when: mouseArea.drag.active
                            ParentChange {
                                target: icon
                                parent: doc
                            }

                            AnchorChanges {
                                target: icon;
                                anchors.horizontalCenter: undefined;
                                anchors.verticalCenter: undefined
                            }
                        }
                        ]

                        Image {
                            id: iconImage
                            sourceSize.width: listView.width-2*Screen.pixelDensity
                            anchors {
                                horizontalCenter: parent.horizontalCenter;
                                top: parent.top
                            }
                            source:  "file://"+model.name
                        }
                        Text {
                            id: number
                            anchors {
                                bottom: parent.bottom
                                horizontalCenter: parent.horizontalCenter
                            }
                            text: model.index+1
                        }
                    }
                    Rectangle {
                        anchors.fill: parent
                        z: -1
                        color: palette.highlight
                        visible: index === listView.currentIndex
                    }
                }
            }
        }

        ColumnLayout {
            SplitView.fillWidth: true
            SplitView.fillHeight: true

            ScrollView {
                id: imageViewer
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                contentWidth: Math.max(bigImage.width, imageViewer.availableWidth)
                contentHeight: Math.max(bigImage.height, imageViewer.availableHeight)
                
                Item {
                    anchors.fill: parent
                    
                    Image {
                        id: bigImage
                        
                        anchors.centerIn: parent

                        property double zoomScale: 1
                        width: sourceSize.width * zoomScale
                        height: sourceSize.height * zoomScale
                    }   
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                visible: skanPage.progress === 100
                
                ToolButton {
                    action: zoomInAction 
                }
                
                ToolButton { 
                    action: zoomOutAction 
                }
                
                ToolButton { 
                    action: zoomFitAction
                }
                
                ToolButton { 
                    action: zoomOrigAction
                }
                
                Item { 
                    id: toolbarSpacer
                    Layout.fillWidth: true
                }
            }
            
            RowLayout {
                Layout.fillWidth: true
                visible: skanPage.progress < 100
                
                ProgressBar {
                    Layout.fillWidth: true
                    value: skanPage.progress / 100
                }
                
                ToolButton { 
                    action: cancelAction
                }
            }
        }
    }

    Keys.onUpPressed: {
        if (event.modifiers & Qt.ControlModifier) {
            moveUp()
        }
        else {
            listView.decrementCurrentIndex()
        }
        event.accepted = true;
    }

    Keys.onDownPressed: {
        if (event.modifiers & Qt.ControlModifier) {
            moveDown()
        }
        else {
            listView.incrementCurrentIndex()
        }
        event.accepted = true;
    }

    function moveUp() {
        if (listView.currentIndex > 0) {
            pages.moveImage(listView.currentIndex, listView.currentIndex-1, 1);
            listView.currentIndex--;
            listView.positionViewAtIndex(listView.currentIndex, ListView.Center);
        }
    }
    
    function moveDown() {
        if (listView.currentIndex < listView.count-1) {
            pages.moveImage(listView.currentIndex, listView.currentIndex+1, 1);
            listView.currentIndex++;
            listView.positionViewAtIndex(listView.currentIndex, ListView.Center);
        }
    }

    Action {
        id: zoomInAction
        icon.name: "zoom-in"
        text: i18n("Zoom In")
        shortcut: StandardKey.ZoomIn
        onTriggered: bigImage.zoomScale *= 1.5
        enabled: bigImage.zoomScale < 8
    }

    Action {
        id: zoomOutAction
        icon.name: "zoom-out"
        text: i18n("Zoom Out")
        shortcut: StandardKey.ZoomOut
        onTriggered: bigImage.zoomScale *= 0.75
        enabled: bigImage.width > imageViewer.availableWidth / 2
    }

    Action {
        id: zoomFitAction
        icon.name: "zoom-fit-best"
        text: i18n("Zoom Fit Width")
        shortcut: "A"
        onTriggered: bigImage.zoomScale = imageViewer.availableWidth / bigImage.sourceSize.width
    }

    Action {
        id: zoomOrigAction
        icon.name: "zoom-original"
        text: i18n("Zoom 100%")
        shortcut: "F"
        onTriggered:  bigImage.zoomScale = 1
    }

    Action {
        id: cancelAction
        icon.name: "window-close"
        text: i18n("Cancel")
        shortcut: "Esc"
        onTriggered: skanPage.cancelScan()
    }

    Action {
        id: deletePageAction
        icon.name: "document-close"
        text: i18n("Delete page")
        shortcut: StandardKey.Delete
        onTriggered: {
            // FIXME ask for confirmation + do not ask again
            delTmr.delIndex = listView.currentIndex;
            delTmr.restart();
        }
    }

    Menu {
        id: myContextMenu
        title: i18n("Edit")

        MenuItem { 
            action: deletePageAction
        }

        MenuItem {
            text: i18n("Move Up")
            onTriggered: moveUp()
        }
        
        MenuItem {
            text: i18n("Move Down")
            onTriggered: moveDown()
        }
    }
}
