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
    id: doc

    focus: true
    clip: true
    
    readonly property string name: skanPage.documentModel.changed ? i18nc("Prefix for document name indicating an unsaved document", "* %1", skanPage.documentModel.name) : skanPage.documentModel.name;

    SystemPalette {
        id: palette
    }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        handle: Kirigami.Separator {
        }
        
        DocumentList {
            id: scrollView

            visible: skanPage.documentModel.count > 1
            
            SplitView.fillHeight: true
            SplitView.preferredWidth: parent.width / 4
            SplitView.minimumWidth: parent.width / 5
        }
                
        Item {
            id: container
    
            SplitView.fillWidth: true
            SplitView.fillHeight: true
    
            Kirigami.InlineMessage {
                id: errorMessage
                width: labelWidth.width + Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing * 2
                height: Math.max(labelWidth.height, Kirigami.Units.iconSizes.medium) + Kirigami.Units.largeSpacing
                type: Kirigami.MessageType.Error
                z: 2
                
                text: skanPage.errorMessage
                
                anchors {
                    top: container.top
                    topMargin: Kirigami.Units.smallSpacing
                    horizontalCenter: container.horizontalCenter
                }
                
                Timer {
                    id: hideNotificationTimer
                    interval: 5000
                    onTriggered: errorMessage.visible = false
                }
                    
                Connections {
                    target: skanPage
                    function onErrorMessageChanged() {
                        errorMessage.text = skanPage.errorMessage
                        labelWidth.text = skanPage.errorMessage
                        errorMessage.visible = true
                        hideNotificationTimer.start()   
                    }
                }  
                
                TextMetrics {
                    id: labelWidth
                }
            }
            
            DocumentPage {
                id: activeDocument
        
                anchors.fill: parent
            }
        }
    }
}
