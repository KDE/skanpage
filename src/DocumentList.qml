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
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami

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

    Keys.onUpPressed: {
        listView.decrementCurrentIndex()
    }

    Keys.onDownPressed: {
        listView.incrementCurrentIndex()
    }
}
