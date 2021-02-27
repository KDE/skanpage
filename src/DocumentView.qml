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
        
        DocumentList {
            id: scrollView

            visible: skanPage.documentModel.count > 1
            
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            
            SplitView.fillHeight: true

            SplitView.preferredWidth: parent.width / 4
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
