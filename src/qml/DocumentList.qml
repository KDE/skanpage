/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import org.kde.kirigami as Kirigami
import org.kde.skanpage 1.0

ColumnLayout {
    id: documentList

    signal saveSinglePage(int pageNumber)
    signal saveSinglePageAs(int pageNumber)

    spacing: 0
    property int minimumWidth: {
        let res = listFooter.implicitWidth
        for (var i = 0; i < listView.count; i++) {
            const el = listView.itemAtIndex(i)
            if (el !== null) {
                const w = el.implicitWidth + scrollView.ScrollBar.vertical.width
                if (w > res) res = w
            }
        }
        return res
    }

    //copied from Kirigami.Separator
    property var midColor: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, 0.15)

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

            clip: true

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

            ShortcutsAction {
                id: selectUpAction
                text: i18n("Select Previous Page")
                shortcut: "UP"
                onTriggered: skanpage.documentModel.activePageIndex--
            }
            ShortcutsAction {
                id: selectDownAction
                text: i18n("Select Next Page")
                shortcut: "DOWN"
                onTriggered: skanpage.documentModel.activePageIndex++
            }
            ShortcutsAction {
                id: moveUpAction
                text: i18n("Move Selected Page Up")
                shortcut: "CTRL+SHIFT+UP"
                onTriggered: {
                    const i = skanpage.documentModel.activePageIndex
                    skanpage.documentModel.moveImage(i, i - 1);
                }
            }
            ShortcutsAction {
                id: moveDownAction
                text: i18n("Move Selected Page Down")
                shortcut: "CTRL+SHIFT+DOWN"
                onTriggered: {
                    const i = skanpage.documentModel.activePageIndex
                    skanpage.documentModel.moveImage(i, i + 1);
                }
            }

            delegate: MouseArea {
                id: mouseArea
                width: Math.max(listView.width - scrollView.ScrollBar.vertical.width, delegateRoot.implicitWidth)
                implicitWidth: delegateRoot.implicitWidth
                implicitHeight: delegateRoot.implicitHeight

                property int actualIndex: index // Needs to be redeclared so it shows up from the dragged-into item

                drag.target: delegateRoot
                drag.axis: Drag.YAxis

                DropArea {
                    anchors.fill: parent
                    onEntered: function(drag) {
                        if (drag.source.actualIndex !== mouseArea.actualIndex)
                            skanpage.documentModel.moveImage(drag.source.actualIndex, mouseArea.actualIndex)
                    }
                }

                onClicked: {
                    skanpage.documentModel.activePageIndex = index
                }

                Control {
                    id: delegateRoot

                    readonly property bool landscape: model.rotationAngle === 270 || model.rotationAngle === 90
                    focus: index === skanpage.documentModel.activePageIndex

                    width: mouseArea.width
                    padding: 2

                    Drag.active: mouseArea.drag.active
                    Drag.source: mouseArea
                    Drag.hotSpot: Qt.point(width / 2, height / 2)

                    states: [
                        State {
                            name: ""
                            ParentChange {
                                target: delegateRoot
                                parent: mouseArea // Reset parent
                            }
                            PropertyChanges {
                                target: delegateRoot
                                x: 0; y: 0 // Snap back to parent's origin (they are the same size)
                            }
                        },
                        State {
                            name: "dragging"
                            when: mouseArea.drag.active
                            ParentChange {
                                target: delegateRoot
                                parent: listView // Lift the item up so it's always visible
                            }
                        }
                    ]

                    background: Rectangle {
                        color: Kirigami.Theme.backgroundColor
                        border.width: delegateRoot.padding
                        border.color: delegateRoot.focus ? Kirigami.Theme.focusColor : documentList.midColor
                        radius: 3
                    }

                    contentItem: ColumnLayout {
                        id: contentColumn

                        Item {
                            Layout.fillWidth: true
                            implicitHeight: delegateRoot.landscape ?
                                            width / model.aspectRatio : width * model.aspectRatio
                            Image {
                                visible: model.isSaved
                                width:  delegateRoot.landscape ? parent.height : parent.width
                                height: delegateRoot.landscape ? parent.width  : parent.height
                                anchors.centerIn: parent

                                source: model.imageUrl
                                sourceSize.height: model.previewHeight
                                sourceSize.width: model.previewWidth

                                rotation: model.rotationAngle
                                asynchronous: true
                            }

                            ColumnLayout {
                                visible: !model.isSaved
                                anchors.fill: parent

                                BusyIndicator {
                                    running: visible

                                    Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                                    Layout.preferredHeight: Layout.preferredWidth
                                    Layout.maximumHeight: parent.height
                                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                }

                                Kirigami.PlaceholderMessage {
                                    visible: parent.height > Kirigami.Units.iconSizes.huge + height
                                    Layout.maximumWidth: parent.width
                                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                    text: xi18nc("@info", "Processing page…")
                                }
                            }
                        }

                        RowLayout {
                            id: bottomRow
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.smallSpacing

                            Kirigami.Heading {
                                Layout.alignment: Qt.AlignLeft
                                level: 2
                                text: i18nc("Page index", "Page %1", model.index + 1)
                            }

                            Kirigami.ActionToolBar {
                                Layout.alignment: Qt.AlignRight
                                alignment: Qt.AlignRight
                                flat: false
                                display: Button.IconOnly
                                actions: [
                                    Kirigami.Action {
                                        icon.name: "go-up"
                                        text: i18n("Move Up")
                                        onTriggered: {
                                            skanpage.documentModel.moveImage(index, index - 1);
                                            listView.positionViewAtIndex(index, ListView.Center);
                                        }
                                        enabled: index > 0
                                    },

                                    Kirigami.Action {
                                        icon.name: "go-down"
                                        text: i18n("Move Down")
                                        onTriggered: {
                                            skanpage.documentModel.moveImage(index, index + 1);
                                            listView.positionViewAtIndex(index, ListView.Center);
                                        }
                                        enabled: index < listView.count - 1
                                    },

                                    Kirigami.Action {
                                        icon.name: "object-rotate-left"
                                        text: i18n("Rotate Left")
                                        onTriggered: skanpage.documentModel.rotateImage(index, DocumentModel.Rotate90negative)
                                    },

                                    Kirigami.Action {
                                        icon.name: "object-rotate-right"
                                        text: i18n("Rotate Right")
                                        onTriggered: skanpage.documentModel.rotateImage(index, DocumentModel.Rotate90positive)
                                    },

                                    Kirigami.Action {
                                        icon.name: "object-flip-vertical"
                                        text: i18n("Flip")
                                        onTriggered: skanpage.documentModel.rotateImage(index, DocumentModel.Flip180)
                                    },

                                    Kirigami.Action {
                                        icon.name: "document-save"
                                        text: i18n("Save Page")
                                        onTriggered: documentList.saveSinglePage(index)
                                    },

                                    Kirigami.Action {
                                        icon.name: "document-save"
                                        text: i18n("Save Page As")
                                        onTriggered: documentList.saveSinglePageAs(index)
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

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    RowLayout {
        id: listFooter
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2

        Label {
            Layout.margins: Kirigami.Units.largeSpacing
            text: i18np("%1 page", "%1 pages", skanpage.documentModel.count)
        }

        Kirigami.ActionToolBar {
            alignment: Qt.AlignRight
            actions: [
                Kirigami.Action {
                    id: flipAction
                    icon.name: "object-flip-vertical"
                    text: i18n("Flip Pages")
                    enabled: skanpage.documentModel.count > 0

                    Kirigami.Action {
                        id: flipOddPagesAction
                        text: i18n("Flip Odd Pages")
                        onTriggered: skanpage.documentModel.flipPages(DocumentModel.FlipOdd)
                        enabled: skanpage.documentModel.count > 1
                    }

                    Kirigami.Action {
                        id: flipEvenPagesAction
                        text: i18n("Flip Even Pages")
                        onTriggered: skanpage.documentModel.flipPages(DocumentModel.FlipEven)
                        enabled: skanpage.documentModel.count > 1
                    }

                    Kirigami.Action {
                        id: flipAllPagesAction
                        text: i18n("Flip All Pages")
                        onTriggered: skanpage.documentModel.flipPages(DocumentModel.FlipAll)
                        enabled: skanpage.documentModel.count > 0
                    }
                },
                Kirigami.Action {
                    id: reorderAction
                    icon.name: "exchange-positions"
                    text: i18n("Reorder Pages")
                    enabled: skanpage.documentModel.count > 1

                    Kirigami.Action {
                        id: reorderDuplexAction
                        text: i18nc("Indicates how pages are going to be reordered", "13 24 → 1234")
                        onTriggered: skanpage.documentModel.reorderPages(DocumentModel.ReorderDuplex)
                        enabled: skanpage.documentModel.count > 2
                    }

                    Kirigami.Action {
                        id: reorderDuplexReverseAction
                        text: i18nc("Indicates how pages are going to be reordered", "13 42 → 1234")
                        onTriggered: skanpage.documentModel.reorderPages(DocumentModel.ReorderDuplexReversed)
                        enabled: skanpage.documentModel.count > 2
                    }

                    Kirigami.Action {
                        id: reverseAction
                        text: i18n("Reverse Order")
                        onTriggered: skanpage.documentModel.reorderPages(DocumentModel.Reverse)
                        enabled: skanpage.documentModel.count > 1
                    }
                }
            ]
        }
    }
}
