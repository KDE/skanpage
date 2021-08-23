/**
 * SPDX-FileCopyrightText: 2020 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import QtQuick.Controls 2.14
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami
import org.kde.skanpage 1.0

Item {
    id: devicesView

    Item {
        id: scanningMessage

        visible: skanpage.applicationState == Skanpage.SearchingForDevices
        anchors.fill: parent

        ColumnLayout {
            anchors.centerIn: parent

            BusyIndicator {
                running: skanpage.applicationState == Skanpage.SearchingForDevices

                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            }

            Kirigami.PlaceholderMessage {

                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                text: xi18nc("@info", "Searching for available devices.")
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        visible: skanpage.applicationState == Skanpage.DeviceSelection

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
                    model: skanpage.devicesModel

                    visible: skanpage.devicesModel.rowCount !== 0

                    spacing: Kirigami.Units.smallSpacing

                    ButtonGroup {
                        id: radioGroup
                    }

                    delegate: RadioButton {
                        id: selectButton

                        checked: index === 0
                        ButtonGroup.group: radioGroup
                        text: i18nc("Device vendor with device model, followed by the device name identifier", "%1 %2\n(%3)", vendor, model, name)

                        onClicked: {
                            selectButton.checked = true
                            skanpage.devicesModel.selectDevice(index)
                        }
                    }
                }
            }

            Kirigami.PlaceholderMessage {
                visible: skanpage.devicesModel.rowCount === 0
                Layout.fillWidth: true

                icon.name: "error"
                text: xi18nc("@info", "No devices found.")
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Row {
            Layout.alignment: Qt.AlignHCenter

            Button {
                id: selectDeviceButton

                visible: skanpage.devicesModel.rowCount !== 0

                icon.name: "select"
                text: i18n("Open selected device")

                onClicked: skanpage.openDevice(skanpage.devicesModel.getSelectedDeviceName())
            }

            Button {
                id: reloadDevicesListButton

                icon.name: "view-refresh"
                text: i18n("Reload devices list")

                onClicked: skanpage.reloadDevicesList()
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}


