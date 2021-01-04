/* ============================================================
 *
 * Copyright (C) 2020 by Alexander Stippich <a.stippich@gmx.net>
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
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami

Item {
    id: devicesView
    
    Item {
        id: scanningMessage

        visible: skanPage.searchingForDevices
        anchors.fill: parent
        
        ColumnLayout {
            anchors.centerIn: parent   
            
            BusyIndicator {
                running: skanPage.searchingForDevices
                
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            }

            Kirigami.PlaceholderMessage {

                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                text: xi18nc("@info", "Scanning for available devices.")
            }   
        }
    }

    ColumnLayout {
        anchors.fill: parent
        visible: !skanPage.searchingForDevices 
        
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            Item {
                Layout.fillWidth: true
            }
            
            Item {
                Layout.preferredWidth: listView.contentItem.childrenRect.width
                Layout.preferredHeight: listView.contentItem.childrenRect.height
                    
                ListView {
                    id: listView
                    anchors.fill: parent
                    model: skanPage.devicesModel
                    
                    ButtonGroup {
                        id: radioGroup
                    }
                
                    delegate: RadioButton {
                        id: selectButton

                        checked: index === 0
                        ButtonGroup.group: radioGroup
                        text: vendor + " " + model;
                        
                        onClicked: {
                            selectButton.checked = true
                            skanPage.devicesModel.selectDevice(index)
                        }
                    }
                }
            }
            
            Item {
                Layout.fillWidth: true
            }
        }

        Row {
            Layout.alignment: Qt.AlignHCenter
            
            Button {
                id: selectDeviceButton
                
                icon.name: "select"
                text: i18n("Open selected device")
                
                onClicked: skanPage.openDevice(skanPage.devicesModel.getSelectedDeviceName())
            }
        
        /*reloading devices with custom GUI currently not possible with libksane
            Button {
                id: reloadDevicesListButton
                
                icon.name: "view-refresh"
                text: i18n("Reload devices list")
                
                onClicked: skanPage.reloadDevicesList()
            }*/
        }
        
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}


