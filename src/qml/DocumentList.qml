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

ColumnLayout {
    id: documentList

    signal saveSinglePage(int pageNumber)

    spacing: 0

    ScrollView {
        id: scrollView

        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        ListView {
            id: listView
            anchors.fill: parent

            spacing: Kirigami.Units.smallSpacing

            Connections {
                target: skanpage.documentModel
                function onActivePageChanged() {
                    listView.positionViewAtIndex(skanpage.documentModel.activePageIndex, ListView.Contain)
                }
            }

            displaced: Transition {
                NumberAnimation {
                    properties: "x,y"
                    easing.type: Easing.OutQuad
                }
            }

            model: skanpage.documentModel

            onCurrentItemChanged: skanpage.documentModel.activePageIndex = currentIndex

            delegate: Rectangle {
                id: delegateRoot

                readonly property int contentWidth: width - border.width * 2
                readonly property bool landscape: (model.rotationAngle == 270 || model.rotationAngle == 90)

                width: listView.width - scrollView.ScrollBar.vertical.width
                height: (landscape ? contentWidth / model.aspectRatio : contentWidth * model.aspectRatio) + bottomRow.height + Kirigami.Units.smallSpacing * 2 + border.width * 2

                color: Kirigami.Theme.backgroundColor

                border.width: 3
                border.color: (index === skanpage.documentModel.activePageIndex) ? Kirigami.Theme.focusColor : palette.mid
                radius: 3

                focus: index === skanpage.documentModel.activePageIndex

                MouseArea {
                    id: mouseArea

                    anchors.fill: parent
                    drag.target: contentColumn

                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton

                    onClicked: skanpage.documentModel.activePageIndex = index

                    DropArea {
                        anchors.fill: parent
                        onEntered: skanpage.documentModel.moveImage(drag.source.index, delegateRoot.index, 1)
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
                            visible: !model.isSaved

                            implicitWidth: contentWidth
                            implicitHeight: contentWidth * model.aspectRatio
                            
                            ColumnLayout {
                                anchors.centerIn: parent

                                BusyIndicator {
                                    running: parent.parent.visible

                                    Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                                    Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                }

                                Kirigami.PlaceholderMessage {
                                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                    text: xi18nc("@info", "Processing page...")
                                }
                            }
                        }
                        
                        Item {
                            visible: model.isSaved
                            
                            implicitWidth: contentWidth
                            implicitHeight: delegateRoot.landscape ? iconImage.width : iconImage.height

                            Image {
                                id: iconImage

                                anchors {
                                    horizontalCenter: parent.horizontalCenter
                                    verticalCenter: parent.verticalCenter
                                }

                                source: model.imageUrl
                                sourceSize.height: model.previewHeight
                                sourceSize.width: model.previewWidth

                                width: delegateRoot.landscape ? contentWidth / model.aspectRatio: contentWidth
                                height: delegateRoot.landscape ? contentWidth : contentWidth * model.aspectRatio

                                transformOrigin: Item.Center
                                rotation: model.rotationAngle
                                asynchronous: true
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

                            Kirigami.ActionToolBar {

                                flat: false
                                alignment: Qt.AlignRight
                                display: Button.IconsOnly
                                actions: [
                                    Kirigami.Action {
                                        icon.name: "go-up"
                                        text: i18n("Move Up")
                                        onTriggered: {
                                            skanpage.documentModel.moveImage(index, index - 1, 1);
                                            listView.positionViewAtIndex(index, ListView.Center);
                                        }
                                        enabled: index > 0
                                    },

                                    Kirigami.Action {
                                        icon.name: "go-down"
                                        text: i18n("Move Down")
                                        onTriggered: {
                                            skanpage.documentModel.moveImage(index, index + 1, 1);
                                            listView.positionViewAtIndex(index, ListView.Center);
                                        }
                                        enabled: index < listView.count - 1
                                    },

                                    Kirigami.Action {
                                        icon.name: "object-rotate-left"
                                        text: i18n("Rotate Left")
                                        onTriggered: skanpage.documentModel.rotateImage(index, true)
                                    },

                                    Kirigami.Action {
                                        icon.name: "object-rotate-right"
                                        text: i18n("Rotate Right")
                                        onTriggered: skanpage.documentModel.rotateImage(index, false)
                                    },

                                    Kirigami.Action {
                                        icon.name: "document-save"
                                        text: i18n("Save Page")
                                        onTriggered: documentList.saveSinglePage(index)
                                    },

                                    Kirigami.Action {
                                        icon.name: "delete"
                                        text: i18n("Delete Page")
                                        onTriggered: skanpage.documentModel.removeImage(index)
                                    }
                                ]
                            }
                        }
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2

        Item {
            Layout.fillWidth: true
        }

        Label {
            text: i18np("%1 page", "%1 pages", skanpage.documentModel.count)
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
