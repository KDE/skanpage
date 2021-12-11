/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami

Item {
    id: documentPage

    signal saveSinglePage(int pageNumber)

    Connections {
        target: skanpage.documentModel
        function onActivePageSourceChanged() {
            zoomFitAction.trigger()
        }
    }

    Kirigami.PlaceholderMessage {
        id: emptyDocumentMessage

        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)

        visible: skanpage.documentModel.count === 0

        icon.name: "document"

        text: xi18nc("@info", "You do not have any images in this document.<nl/><nl/>Start scanning!")
    }

    ColumnLayout {
        id: documentLayout

        anchors.fill: parent

        spacing: 0

        visible: skanpage.documentModel.count > 0

        ScrollView {
            id: imageViewer
            Layout.fillWidth: true
            Layout.fillHeight: true

            contentWidth: Math.max(bigImage.width, imageViewer.availableWidth)
            contentHeight: Math.max(bigImage.height, imageViewer.availableHeight)

            Item {
                anchors.centerIn: parent

                implicitWidth: bigImage.landscape ? bigImage.height : bigImage.width
                implicitHeight: bigImage.landscape ? bigImage.width : bigImage.height

                Image {
                    id: bigImage

                    readonly property bool landscape: (rotation == 270 || rotation == 90)
                    property double zoomScale: Math.min(imageViewer.availableWidth / bigImage.sourceSize.width, 1)

                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }

                    width: sourceSize.width * zoomScale
                    height: sourceSize.height * zoomScale

                    source: skanpage.documentModel.activePageSource

                    rotation: skanpage.documentModel.activePageRotation
                    transformOrigin: Item.Center
                }
            }
        }

        Kirigami.ActionToolBar {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2

            alignment: Qt.AlignLeft

            actions: [
                Kirigami.Action {
                    id: zoomInAction
                    icon.name: "zoom-in"
                    text: i18n("Zoom In")
                    shortcut: StandardKey.ZoomIn
                    onTriggered: bigImage.zoomScale *= 1.5
                    enabled: bigImage.zoomScale < 8
                },

                Kirigami.Action {
                    id: zoomOutAction
                    icon.name: "zoom-out"
                    text: i18n("Zoom Out")
                    shortcut: StandardKey.ZoomOut
                    onTriggered: bigImage.zoomScale *= 0.75
                    enabled: bigImage.width > imageViewer.availableWidth / 2
                },

                Kirigami.Action {
                    id: zoomFitAction
                    icon.name: "zoom-fit-best"
                    text: i18n("Zoom Fit")
                    shortcut: "A"
                    onTriggered: {
                        var zoomScaleWidth = imageViewer.availableWidth / bigImage.sourceSize.width
                        var zoomScaleHeight = imageViewer.availableHeight / bigImage.sourceSize.height
                        if (zoomScaleWidth < zoomScaleHeight) {
                            bigImage.zoomScale = zoomScaleWidth
                        } else {
                            bigImage.zoomScale = zoomScaleHeight
                        }
                    }
                },

                Kirigami.Action {
                    id: zoomOrigAction
                    icon.name: "zoom-original"
                    text: i18n("Zoom 100%")
                    shortcut: "F"
                    onTriggered: bigImage.zoomScale = 1
                },

                Kirigami.Action {
                    id: rotateLeftAction
                    icon.name: "object-rotate-left"
                    text: i18n("Rotate Left")
                    onTriggered: skanpage.documentModel.rotateImage(skanpage.documentModel.activeIndex, true)
                },

                Kirigami.Action {
                    id: rotateRightAction
                    icon.name: "object-rotate-right"
                    text: i18n("Rotate Right")
                    onTriggered: skanpage.documentModel.rotateImage(skanpage.documentModel.activeIndex, false)
                },

                Kirigami.Action {
                    id: savePageAction
                    icon.name: "document-save"
                    text: i18n("Save Page")
                    onTriggered: documentPage.saveSinglePage(skanpage.documentModel.activeIndex)
                },

                Kirigami.Action {
                    id: deleteAction
                    icon.name: "delete"
                    text: i18n("Delete Page")
                    onTriggered: skanpage.documentModel.removeImage(skanpage.documentModel.activeIndex)
                }
            ]
        }
    }
}


