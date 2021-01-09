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
import org.kde.kirigami 2.12 as Kirigami

Item {
    id: doc

    focus: true
    clip: true
    
    readonly property string name: skanPage.documentModel.changed ? i18nc("* ", "Marker for indicating an unsaved document") + skanPage.documentModel.name : skanPage.documentModel.name;

    SystemPalette {
        id: palette
        colorGroup: SystemPalette.Inactive 
    }

    Timer {
        id: delTmr
        interval: 1
        property int delIndex
        onTriggered: {
            skanPage.documentModel.removeImage(delIndex);
            if (listView.count === 0) {
                bigImage.source = "";
            }
        }
    }
    
    Item {
        id: emptyDocumentMessage

        visible: skanPage.documentModel.count === 0

        anchors.fill: parent

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)

            icon.name: "document"
            
            text: xi18nc("@info", "You do not have any images in this document.<nl/><nl/>Start scanning!")
        }
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        visible: skanPage.documentModel.count !== 0
        
        ScrollView {
            id: scrollView

            visible: skanPage.documentModel.count > 1
            
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            
            SplitView.fillHeight: true
            SplitView.preferredWidth: parent.width / 4         

            ListView {
                id: listView
                anchors.fill: parent

                displaced: Transition {
                    NumberAnimation {
                        properties: "x,y"
                        easing.type: Easing.OutQuad 
                    }
                }

                model: skanPage.documentModel

                onCurrentItemChanged: {
                    bigImage.source = listView.currentItem ? listView.currentItem.imageUrl : ""
                    bigImage.zoomScale = Math.min(imageViewer.availableWidth / bigImage.sourceSize.width, 1)
                }

                delegate: Rectangle {
                    id: delegateRoot
                    readonly property int borderWidth: 2
                    border.width: borderWidth
                    border.color: palette.mid

                    focus: index === listView.currentIndex

                    property url imageUrl: model.imageUrl
                    property int index: model.index
                    
                    width: listView.width;
                    height: icon.height

                    MouseArea {
                        id: mouseArea
                        
                        anchors.fill: parent
                        drag.target: icon
                        
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton

                        onClicked: {
                            listView.currentIndex = index;
                        }
                    }

                    DropArea {
                        anchors.fill: parent
                        onEntered: {
                            skanPage.documentModel.moveImage(drag.source.index, delegateRoot.index, 1);
                        }
                    }

                    Item {
                        id: icon
                        width: iconImage.width
                        height: iconImage.height + pageNumber.height
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
                            sourceSize.width: listView.width - borderWidth * 2
                            anchors {
                                horizontalCenter: parent.horizontalCenter;
                                top: parent.top
                            }
                            source: model.imageUrl
                        }
                        
                        Kirigami.Heading {
                            id: pageNumber
                            level: 2
                            anchors {
                                bottom: parent.bottom
                                horizontalCenter: parent.horizontalCenter
                            }
                            text: model.index + 1
                        }
                        
                        Loader {
                            active: mouseArea.containsMouse
                            
                            anchors {
                                bottom: pageNumber.top
                                left: icon.left
                            }
                            
                            sourceComponent: Row {
                                spacing: Kirigami.Units.smallSpacing
                                
                                Button {
                                    icon.name: "go-up"
                                    onClicked: {
                                        skanPage.documentModel.moveImage(listView.currentIndex, listView.currentIndex -1, 1);
                                        listView.currentIndex--;
                                        listView.positionViewAtIndex(listView.currentIndex, ListView.Center);
                                    }
                                    enabled: index > 0 
                                }
                                
                                Button {
                                    icon.name: "go-down"
                                    onClicked: {
                                        skanPage.documentModel.moveImage(listView.currentIndex, listView.currentIndex + 1, 1);
                                        listView.currentIndex++;
                                        listView.positionViewAtIndex(listView.currentIndex, ListView.Center);
                                    }
                                    enabled: index < listView.count - 1
                                }
                                
                                Button {      
                                    icon.name: "delete"
                                    onClicked: {
                                        skanPage.documentModel.removeImage(index);
                                    }
                                }
                            }
                        }
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
        listView.decrementCurrentIndex()
    }

    Keys.onDownPressed: {
        listView.incrementCurrentIndex()
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
}
