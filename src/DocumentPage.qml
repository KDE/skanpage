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

ColumnLayout {
    id: documentLayout
    
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
        onTriggered: skanPage.documentModel.rotateImage(skanPage.documentModel.activeIndex, true)
    }

    Action {
        id: rotateRightAction
        icon.name: "object-rotate-right"
        text: i18n("Rotate Right")
        onTriggered:  skanPage.documentModel.rotateImage(skanPage.documentModel.activeIndex, false)
    }

    Action {
        id: deleteAction
        icon.name: "delete"
        text: i18n("Delete Page")
        onTriggered: skanPage.documentModel.removeImage(skanPage.documentModel.activeIndex)
    }
    
        
    Item {
        id: emptyDocumentMessage

        visible: skanPage.documentModel.count === 0

        Layout.fillWidth: true
        Layout.fillHeight: true

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)

            icon.name: "document"
            
            text: xi18nc("@info", "You do not have any images in this document.<nl/><nl/>Start scanning!")
        }
    }
    
    ScrollView {
        id: imageViewer
        Layout.fillWidth: true
        Layout.fillHeight: true
        
        visible: skanPage.documentModel.count !== 0
        
        contentWidth: Math.max(bigImage.width, imageViewer.availableWidth)
        contentHeight: Math.max(bigImage.height, imageViewer.availableHeight)
        
        Item {
            anchors.fill: parent
            
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
                
                source: skanPage.documentModel.activePageSource
                
                rotation: skanPage.documentModel.activePageRotation
                transformOrigin: Item.Center
            }     
        }
    }
    
    RowLayout {
        Layout.fillWidth: true
        visible: skanPage.progress === 100 && skanPage.documentModel.count !== 0
        
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

  
