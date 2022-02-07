/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami
import org.kde.skanpage 1.0

ColumnLayout {
    id: optionPanel

    property alias showAllOptions: allOptionsButton.checked
    property int targetWidth: Math.max(_maxChildrenWidth + optionsList.ScrollBar.vertical.width,
                                       allOptionsButton.implicitWidth, devicesButton.implicitWidth)
    property int _maxChildrenWidth: 0
    
    property bool editMode: false

    Label {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
        text: i18n("Select options for quick access:")
        visible: editMode
    }

    ScrollView {
        id: optionsList

        Layout.fillWidth: true
        Layout.fillHeight: true

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ListView {
            model: skanpage.optionsModel

            enabled: skanpage.applicationState == Skanpage.ReadyForScan
            
            delegate: OptionDelegate {
                modelItem: model
                
                width: optionsList.width - optionsList.ScrollBar.vertical.width
                
                editMode: optionPanel.editMode
                Component.onCompleted: {
                    if (optionPanel._maxChildrenWidth < implicitWidth) {
                        optionPanel._maxChildrenWidth = implicitWidth
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter

        Button {
            id: allOptionsButton
            action: allOptionsAction
            enabled: !editOptionsButton.checked
        }

        Button {
            id: editOptionsButton
            icon.name: "settings-configure"
            text: i18n("Configure")
            checkable: true
            onClicked: {
                optionPanel.editMode = checked
                skanpage.optionsModel.showAllOptions(checked)
            }
        }
    }

    Button {
        id: devicesButton
        Layout.alignment: Qt.AlignHCenter
        action: reselectDevicesAction
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
        text: skanpage.deviceVendor && skanpage.deviceModel ?
        i18nc("scanner device vendor and model", "%1 %2", skanpage.deviceVendor, skanpage.deviceModel) : ""
    }
}
