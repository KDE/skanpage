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
    id: previewScan

    function getScanArea() {
        return Qt.rect(selection.selectionX / selection.width, selection.selectionY / selection.height, selection.selectionWidth / selection.width, selection.selectionHeight / selection.height)
    }

    signal closeView()

    component TransparentButton: RoundButton {
        visible: action ? action.enabled : true
        anchors.margins: Kirigami.Units.smallSpacing
        display: AbstractButton.IconOnly
        Kirigami.Theme.colorSet: Kirigami.Theme.Button
        hoverEnabled: true
        opacity: {
            if (hovered) return 1.0 // When using a mouse, visible on hover
            if (Kirigami.Settings.tabletMode || Kirigami.Settings.hasTransientTouchInput) return 0.75
            return 0.5 // When using a mouse, barely visible otherwise, to not get in the way
        }
        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.smallDuration
            }
        }
        ToolTip.visible: hovered
        ToolTip.delay: Kirigami.Units.toolTipDelay
        ToolTip.text: action ? action.text : ""
    }

    InProgressPage {
        id: inProgressPreview
        anchors.fill: parent
        visible: skanpage.applicationState === Skanpage.ScanInProgress
    }

    KQuickImageEditor.ImageItem {
        id: previewImage

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: selectionToolbar.top
        }
        anchors.margins: Kirigami.Units.largeSpacing
        fillMode: KQuickImageEditor.ImageItem.PreserveAspectFit
        smooth: true // The classic fuzzyness of low res
        image: skanpage.previewImage
        visible: skanpage.previewImageAvailable && !inProgressPreview.visible

        KQuickImageEditor.SelectionTool {
            id: selection
            visible: skanpage.scanArea.width > 0
            x: previewImage.horizontalPadding + 1
            y: previewImage.verticalPadding + 1
            width: previewImage.paintedWidth
            height: previewImage.paintedHeight

            property bool pressed: pressedHandle || selectionArea.pressed
            onPressedChanged: if (!pressed) skanpage.scanArea = getScanArea()

            // A shortcut for scanning the full area seems useful
            selectionArea.onDoubleClicked: { // It is updated then releasing the mouse
                skanpage.clearSubAreas()
                selectionX = 0; selectionY = 0; selectionWidth = width; selectionHeight = height
            }

            MouseArea {
                id: newAreaSelector
                anchors.fill: parent
                anchors.rightMargin: selection.state === "queueNewArea"
                                   ? Kirigami.Units.gridUnit : selection.selectionWidth
                anchors.bottomMargin: selection.state === "queueNewArea"
                                    ? Kirigami.Units.gridUnit : selection.selectionHeight
                property bool active: selection.state === "queueNewArea" ||
                                      selection.state === "addingNewArea" || selection.state === "sizingNewArea"
                z: active ? 1 : -1

                onPressed: if (active) {
                    selection.state = "sizingNewArea"
                }
                onReleased: if (active) {
                    skanpage.scanArea = getScanArea()
                    // Add the area as a sub-area to remove nested sub-areas and too overlaping ones
                    if (skanpage.appendSubArea(skanpage.scanArea)) // If the area was added
                        if (skanpage.scanSubAreas.length >= 1)
                            skanpage.eraseSubArea(skanpage.scanSubAreas.length - 1) // Remove last
                    selection.state = "idle"
                }
                HoverHandler {
                    // This seems to be more reliable than the MouseArea containsMouse, apparently
                    // that doesn't update immediately after changing its z value
                    id: newAreaSensor
                    onHoveredChanged: if (hovered) { // onEntered the area
                        if (selection.state === "queueNewArea") selection.state = "addingNewArea"
                    }
                }
            }

            property point dragStart
            property point dragEnd

            states: [
                State {
                    name: "idle"
                    when: !selection.pressed
                    PropertyChanges {
                        restoreEntryValues: false
                        target: selection
                        selectionX: skanpage.scanArea.x * selection.width
                        selectionY: skanpage.scanArea.y * selection.height
                        selectionWidth: skanpage.scanArea.width * selection.width
                        selectionHeight: skanpage.scanArea.height * selection.height
                    }
                },
                State {
                    name: ""
                    PropertyChanges {
                        explicit: true
                        target: selection
                        selectionX: undefined // Manual control
                        selectionY: undefined // Manual control
                        selectionWidth: undefined // Manual control
                        selectionHeight: undefined // Manual control
                    }
                },
                State { // Only used for the transition, especially if the mouse is outside the image
                    name: "queueNewArea"
                    extend: "idle"
                },
                State {
                    name: "addingNewArea"
                    PropertyChanges {
                        restoreEntryValues: false
                        target: selection
                        selectionX: newAreaSensor.point.position.x
                        selectionY: newAreaSensor.point.position.y
                        selectionWidth: Kirigami.Units.gridUnit
                        selectionHeight: Kirigami.Units.gridUnit
                    }
                },
                State {
                    name: "sizingNewArea"
                    PropertyChanges {
                        explicit: true
                        target: selection
                        dragStart: Qt.point(selectionX, selectionY)
                    }
                    PropertyChanges {
                        restoreEntryValues: false
                        target: selection
                        dragEnd: Qt.point(Math.min(Math.max(newAreaSelector.mouseX, 0), selection.width),
                                          Math.min(Math.max(newAreaSelector.mouseY, 0), selection.height))
                        selectionX: dragEnd.x > dragStart.x ? dragStart.x : dragEnd.x
                        selectionY: dragEnd.y > dragStart.y ? dragStart.y : dragEnd.y
                        selectionWidth:  Math.max(Math.abs(dragEnd.x - dragStart.x), Kirigami.Units.gridUnit)
                        selectionHeight: Math.max(Math.abs(dragEnd.y - dragStart.y), Kirigami.Units.gridUnit)
                    }
                }
            ]

            KQuickImageEditor.CropBackground {
                anchors.fill: parent
                insideX: selection.selectionX
                insideY: selection.selectionY
                insideWidth: selection.selectionWidth
                insideHeight: selection.selectionHeight
            }

            Rectangle {
                x: selection.selectionX + selection.selectionWidth / 2 - 1
                y: selection.selectionY
                width: 2
                height: selection.selectionHeight
                color: Kirigami.Theme.highlightColor
                visible: skanpage.scanSplit === Skanpage.ScanIsSplitV
            }

            Rectangle {
                x: selection.selectionX
                y: selection.selectionY + selection.selectionHeight / 2 - 1
                width: selection.selectionWidth
                height: 2
                color: Kirigami.Theme.highlightColor
                visible: skanpage.scanSplit === Skanpage.ScanIsSplitH
            }

            Repeater {
                z: 1
                model: skanpage.scanSubAreas
                delegate: Rectangle {
                    id: subAreaGraphic
                    z: 1
                    color: "transparent"
                    border.color: Kirigami.Theme.focusColor
                    border.width: 2
                    x: modelData.x * selection.width
                    y: modelData.y * selection.height
                    width: modelData.width * selection.width
                    height: modelData.height * selection.height

                    MouseArea {
                        z: -1 // Give priority to the main area
                        parent: selection // To make z work
                        x: subAreaGraphic.x; width:  subAreaGraphic.width
                        y: subAreaGraphic.y; height: subAreaGraphic.height
                        onClicked: skanpage.selectSubArea(index)
                    }
                    TransparentButton {
                        anchors {
                            top:   parent.top; right: parent.right
                            topMargin:   Math.min(Math.max(
                                parent.height - implicitHeight, 0), Kirigami.Units.smallSpacing)
                            rightMargin: Math.min(Math.max(
                                parent.width/2 - implicitWidth, 0), Kirigami.Units.smallSpacing)
                        }
                        width: anchors.rightMargin === 0 ? parent.width / 2 : implicitWidth
                        height: anchors.topMargin  === 0 ? parent.height    : implicitHeight
                        z: 1
                        icon.name: anchors.rightMargin === 0 || anchors.topMargin === 0 ? "" : "edit-delete-remove"
                        onClicked: skanpage.eraseSubArea(index)
                        ToolTip.text: i18n("Discard this selection")
                    }
                }
            }
        }
    }

    Kirigami.ActionToolBar {
        id: selectionToolbar

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: Kirigami.Units.gridUnit * 2

        alignment: Qt.AlignCenter

        actions: [
            ShortcutsAction {
                id: newPreviewScanAction
                icon.name: "document-preview"
                text: i18n("New Preview Scan")
                shortcut: "A"
                enabled: skanpage.applicationState === Skanpage.ReadyForScan
                onTriggered: skanpage.previewScan()
            },

            ShortcutsAction {
                id: addSubAreaAction
                icon.name: "document-open-recent"
                text: i18n("Add Selection Area")
                shortcut: "Q"
                onTriggered: {
                    skanpage.newUserMessage(SkanpageUtils.InformationMessage,
                                            i18n("Click and drag to select another area"))
                    skanpage.appendSubArea(getScanArea())
                    selection.state = "queueNewArea"
                    if (newAreaSensor.hovered) selection.state = "addingNewArea"
                }
            },

            ShortcutsAction {
                id: splitVerticalAction
                icon.name: "view-split-left-right"
                text: i18n("Split Scan Vertically")
                shortcut: "V"
                checkable: true
                checked: skanpage.scanSplit === Skanpage.ScanIsSplitV
                onTriggered: skanpage.scanSplit = checked ? Skanpage.ScanIsSplitV : Skanpage.ScanNotSplit
            },

            ShortcutsAction {
                id: splitHorizontalAction
                icon.name: "view-split-top-bottom"
                text: i18n("Split Scan Horizontally")
                shortcut: "H"
                checkable: true
                checked: skanpage.scanSplit === Skanpage.ScanIsSplitH
                onTriggered: skanpage.scanSplit = checked ? Skanpage.ScanIsSplitH : Skanpage.ScanNotSplit
            },

            ShortcutsAction {
                id: showPreviewAction
                icon.name: "dialog-close"
                text: i18n("Close Preview")
                shortcut: "Esc"
                onTriggered: previewScan.closeView()
            }
        ]
    }
}
