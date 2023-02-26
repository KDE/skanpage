/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kquickimageeditor 1.0 as KQuickImageEditor
import org.kde.skanpage 1.0

Item {
    id: documentPage

    signal saveSinglePage(int pageNumber)

    Connections {
        target: skanpage.documentModel

        function onActivePageSourceChanged() {
            zoomFitAction.trigger()
        }
    }

    property bool showPreview: false

    function getScanArea() {
        return Qt.rect(selection.selectionX / selection.width, selection.selectionY / selection.height, selection.selectionWidth / selection.width, selection.selectionHeight / selection.height)
    }

    component CornerCloseButton: RoundButton {
        visible: action ? action.enabled : true
        anchors.right: parent.right
        anchors.top: parent.top
        readonly property bool needSpace: parent.height < implicitHeight + 2*Kirigami.Units.smallSpacing ||
                                          parent.width  < implicitWidth  + 2*Kirigami.Units.smallSpacing
        anchors.margins: needSpace ? 0 : Kirigami.Units.smallSpacing
        display: AbstractButton.IconOnly
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        hoverEnabled: true
        opacity: hovered ? 1.0 : 0.5
        width: needSpace ? Math.min(implicitWidth, parent.width) : implicitWidth
        height: needSpace ? Math.min(implicitHeight, parent.height) : implicitHeight
    }

    Action {
        id: showPreviewAction
        icon.name: "dialog-close"
        text: i18n("Show Preview")
        shortcut: "Esc"
        enabled: !previewImage.null
        onTriggered: showPreview = true
    }

    Kirigami.PlaceholderMessage {
        id: emptyDocumentMessage

        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)

        visible: skanpage.documentModel.count === 0 && !showPreview && previewImage.null

        icon.name: "document"

        text: xi18nc("@info", "You do not have any images in this document.<nl/><nl/>Start scanning!")
    }

    KQuickImageEditor.ImageItem {
        id: previewImage
        visible: (skanpage.documentModel.count === 0 || showPreview) && !previewImage.null
        enabled: visible

        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit
        fillMode: KQuickImageEditor.ImageItem.PreserveAspectFit
        smooth: true // The classic fuzzyness of low res
        image: skanpage.previewImage

        KQuickImageEditor.SelectionTool {
            id: selection
            visible: skanpage.scanArea.width > 0
            enabled: visible
            x: previewImage.horizontalPadding + 1
            y: previewImage.verticalPadding + 1
            width: previewImage.paintedWidth
            height: previewImage.paintedHeight

            property bool pressed: pressedHandle || selectionArea.pressed
            onPressedChanged: if (!pressed) skanpage.scanArea = getScanArea()

            // A shortcut for scanning the full area seems useful
            selectionArea.onDoubleClicked: { // It is updated then releasing the mouse
                selectionX = 0; selectionY = 0; selectionWidth = width; selectionHeight = height
            }
            // Binding types: to keep the selection area propertional to the image, not constant pixel values
            Binding on selectionX {
                value: skanpage.scanArea.x * selection.width
                when: !selection.pressed; restoreMode: Binding.RestoreNone
            }
            Binding on selectionY {
                value: skanpage.scanArea.y * selection.height
                when: !selection.pressed; restoreMode: Binding.RestoreNone
            }
            Binding on selectionWidth {
                value: skanpage.scanArea.width * selection.width
                when: !selection.pressed; restoreMode: Binding.RestoreNone
            }
            Binding on selectionHeight {
                value: skanpage.scanArea.height * selection.height
                when: !selection.pressed; restoreMode: Binding.RestoreNone
            }

            KQuickImageEditor.CropBackground {
                anchors.fill: parent
                insideX: selection.selectionX
                insideY: selection.selectionY
                insideWidth: selection.selectionWidth
                insideHeight: selection.selectionHeight
            }
        }
    }

    ColumnLayout {
        id: documentLayout

        anchors.fill: parent

        spacing: 0

        visible: skanpage.documentModel.count > 0 && (!showPreview || previewImage.null)

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

                    CornerCloseButton {
                        action: showPreviewAction
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        ToolTip.text: i18n("Show the preview image")
                    }
                }
            }
        }

        Kirigami.ActionToolBar {
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2

            alignment: Qt.AlignLeft

            actions: [
                ShortcutsAction {
                    id: zoomInAction
                    icon.name: "zoom-in"
                    text: i18n("Zoom In")
                    shortcutsName: "ZoomIn"
                    onTriggered: bigImage.zoomScale *= 1.5
                    enabled: bigImage.zoomScale < 8
                },

                ShortcutsAction {
                    id: zoomOutAction
                    icon.name: "zoom-out"
                    text: i18n("Zoom Out")
                    shortcutsName: "ZoomOut"
                    onTriggered: bigImage.zoomScale *= 0.75
                    enabled: bigImage.width > imageViewer.availableWidth / 2
                },

                ShortcutsAction {
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

                ShortcutsAction {
                    id: zoomOrigAction
                    icon.name: "zoom-original"
                    text: i18n("Zoom 100%")
                    shortcut: "F"
                    onTriggered: bigImage.zoomScale = 1
                },

                ShortcutsAction {
                    id: rotateLeftAction
                    icon.name: "object-rotate-left"
                    text: i18n("Rotate Left")
                    onTriggered: skanpage.documentModel.rotateImage(skanpage.documentModel.activePageIndex, DocumentModel.Rotate90positive)
                },

                ShortcutsAction {
                    id: rotateRightAction
                    icon.name: "object-rotate-right"
                    text: i18n("Rotate Right")
                    onTriggered: skanpage.documentModel.rotateImage(skanpage.documentModel.activePageIndex, DocumentModel.Rotate90negative)
                },

                ShortcutsAction {
                    id: flipVerticalAction
                    icon.name: "object-flip-vertical"
                    text: i18n("Flip")
                    onTriggered: skanpage.documentModel.rotateImage(skanpage.documentModel.activePageIndex, DocumentModel.Flip180)
                },

                ShortcutsAction {
                    id: savePageAction
                    icon.name: "document-save"
                    text: i18n("Save Page")
                    onTriggered: documentPage.saveSinglePage(skanpage.documentModel.activePageIndex)
                },

                ShortcutsAction {
                    id: deleteAction
                    icon.name: "delete"
                    text: i18n("Delete Page")
                    shortcut: StandardKey.Delete
                    onTriggered: skanpage.documentModel.removeImage(skanpage.documentModel.activePageIndex)
                }
            ]
        }
    }
}


