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

    property int targetWidth: Math.max(_maxChildrenWidth + optionsList.ScrollBar.vertical.width,
                                       optionsConfiguration.implicitWidth)
    property int _maxChildrenWidth: 0

    property alias allOptionsAction: allOptionsAction
    property alias reselectDevicesAction: reselectDevicesAction
    property alias configureVisibilityAction: configureVisibilityAction

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

    Label {
        Layout.alignment: Qt.AlignHCenter
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
        text: skanpage.deviceVendor && skanpage.deviceModel ?
        i18nc("scanner device vendor and model", "%1 %2", skanpage.deviceVendor, skanpage.deviceModel) : ""
    }

    Kirigami.ActionToolBar {
        id: optionsConfiguration
        flat: false
        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
        alignment: Qt.AlignCenter
        display: Button.IconOnly
        actions: [
            ShortcutsAction {
                id: allOptionsAction
                icon.name: "view-more-symbolic"
                text: i18n("Show More")
                shortcut: "CTRL+SPACE"
                checkable: true
                checked: skanpage.stateConfiguration.showAllOptions
                onTriggered: skanpage.optionsModel.showAllOptions(checked)
            },

            ShortcutsAction {
                id: configureVisibilityAction
                icon.name: "settings-configure"
                text: i18n("Configure Visibility")
                checkable: true
                onTriggered: {
                    optionPanel.editMode = checked
                    if (checked) {
                        skanpage.optionsModel.showAllOptions(checked)
                    } else {
                        skanpage.optionsModel.showAllOptions(allOptionsAction.checked)
                    }
                }
            },
            
            ShortcutsAction {
                id: reselectDevicesAction
                icon.name: "view-refresh"
                text: i18n("Reselect Scanner")
                onTriggered: skanpage.reloadDevicesList()
                enabled: skanpage.applicationState == Skanpage.ReadyForScan
            }
        ]
    }
}
