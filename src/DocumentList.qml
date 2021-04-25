/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.15 
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami

ScrollView {
    id: scrollView
               
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AlwaysOn 
       
    ListView {
        id: listView
        anchors.fill: parent

        spacing: Kirigami.Units.smallSpacing
        
        Connections {
            target: skanPage.documentModel
            function onActivePageChanged() {
                listView.positionViewAtIndex(skanPage.documentModel.activePageIndex, ListView.Contain)
            }
        }
        
        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                easing.type: Easing.OutQuad 
            }
        }

        model: skanPage.documentModel
        
        onCurrentItemChanged: skanPage.documentModel.activePageIndex = currentIndex

        delegate: Rectangle {
            id: delegateRoot
            
            readonly property url imageUrl: model.imageUrl
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

                onClicked: listView.currentIndex = index;

                DropArea {
                    anchors.fill: parent
                    onEntered: skanPage.documentModel.moveImage(drag.source.index, delegateRoot.index, 1);
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

    Keys.onUpPressed: {
        listView.decrementCurrentIndex()
    }

    Keys.onDownPressed: {
        listView.incrementCurrentIndex()
    }
}
