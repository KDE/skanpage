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
import QtQuick.Controls 2.14 
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami

Item {
    id: doc

    focus: true
    clip: true
    
    readonly property string name: skanPage.documentModel.changed ? i18nc("Prefix for document name indicating an unsaved document", "* ") + skanPage.documentModel.name : skanPage.documentModel.name;

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

                spacing: Kirigami.Units.smallSpacing
                
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
                    bigImage.rotation = listView.currentItem ? Qt.binding(function() {return listView.currentItem.rotationAngle}) : 0
                }

                delegate: Rectangle {
                    id: delegateRoot
                    
                    readonly property url imageUrl: model.imageUrl
                    readonly property int index: model.index
                    readonly property int contentWidth: width - border.width * 2
                    readonly property int rotationAngle: model.rotationAngle
                    readonly property bool landscape: (model.rotationAngle == 270 || model.rotationAngle == 90)
                    
                    width: listView.width - scrollView.ScrollBar.vertical.width 
                    height: (landscape ? contentWidth / iconImage.aspectRatio : contentWidth * iconImage.aspectRatio) + bottomRow.height + Kirigami.Units.smallSpacing * 2 + border.width * 2
                    
                    border.width: 3
                    border.color: palette.mid
                    radius: 3

                    focus: index === listView.currentIndex
                    
                    MouseArea {
                        id: mouseArea
                        
                        anchors.fill: parent
                        drag.target: contentColumn
                        
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton

                        onClicked: {
                            listView.currentIndex = index;
                        }
                            
                        DropArea {
                            anchors.fill: parent
                            onEntered: {
                                skanPage.documentModel.moveImage(drag.source.index, delegateRoot.index, 1);

                            }
                        }
                        
                        ColumnLayout {
                            id: contentColumn
                            
                            spacing: 0
                            width: contentWidth
                            
                            anchors {
                                horizontalCenter: parent.horizontalCenter
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
                                        target: contentColumn
                                        parent: doc
                                    }

                                    AnchorChanges {
                                        target: contentColumn
                                        anchors.horizontalCenter: undefined
                                        anchors.verticalCenter: undefined
                                    }
                                }
                            ]
                            
                            Item {
                                implicitWidth: delegateRoot.landscape ? iconImage.height : iconImage.width
                                implicitHeight: delegateRoot.landscape ? iconImage.width : iconImage.height
                                
                                Image {
                                    id: iconImage
      
                                    readonly property real aspectRatio: sourceSize.height / sourceSize.width                  
                                    
                                    anchors {
                                        horizontalCenter: parent.horizontalCenter
                                        verticalCenter: parent.verticalCenter
                                    }               
                                    source: model.imageUrl
                                    
                                    width: delegateRoot.landscape ? contentWidth / aspectRatio: contentWidth
                                    height: delegateRoot.landscape ? contentWidth : contentWidth * aspectRatio
                                    
                                    transformOrigin: Item.Center
                                    rotation: model.rotationAngle
                                }
                            }
                            
                            RowLayout {
                                id: bottomRow
                                Layout.margins: Kirigami.Units.smallSpacing
                                spacing: Kirigami.Units.smallSpacing
                                
                                width: contentWidth
                                
                                Kirigami.Heading {
                                    id: pageNumber
                                    
                                    Layout.alignment: Qt.AlignLeft
                                    
                                    level: 2
                                    text: i18nc("Page index", "Page %1", model.index + 1)
                                }
                                
                                Item {
                                    Layout.fillWidth: true
                                }
                                
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
                                    icon.name: "object-rotate-left"
                                    onClicked: skanPage.documentModel.rotateImage(index, true)
                                }

                                Button {
                                    icon.name: "object-rotate-right"
                                    onClicked: skanPage.documentModel.rotateImage(index, false)
                                }
                                
                                Button {      
                                    icon.name: "delete"
                                    onClicked: skanPage.documentModel.removeImage(index)
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
                    
                    implicitWidth: bigImage.landscape ? bigImage.height : bigImage.width
                    implicitHeight: bigImage.landscape ? bigImage.width : bigImage.height
                    
                    Image {
                        id: bigImage
                        
                        readonly property bool landscape: (rotation == 270 || rotation == 90)
                        
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            verticalCenter: parent.verticalCenter
                        } 

                        property double zoomScale: 1
                        width: sourceSize.width * zoomScale
                        height: sourceSize.height * zoomScale
                        
                        transformOrigin: Item.Center
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
        
                ToolButton { 
                    action: rotateLeftAction 
                }
                
                ToolButton { 
                    action: rotateRightAction
                }
                
                ToolButton { 
                    action: deleteAction
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
    
    Action {
        id: rotateLeftAction
        icon.name: "object-rotate-left"
        text: i18n("Rotate Left")
        onTriggered: skanPage.documentModel.rotateImage(listView.currentIndex, true)
    }

    Action {
        id: rotateRightAction
        icon.name: "object-rotate-right"
        text: i18n("Rotate Right")
        onTriggered:  skanPage.documentModel.rotateImage(listView.currentIndex, false)
    }

    Action {
        id: deleteAction
        icon.name: "delete"
        text: i18n("Delete Page")
        onTriggered: skanPage.documentModel.removeImage(listView.currentIndex)
    }
}
