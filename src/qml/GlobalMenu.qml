/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.7
import Qt.labs.platform 1.1

MenuBar {

    property var newDocAction
    property var saveDocAction
    property var printAction
    property var shareAction
    property var quitAction
    property var previewAction
    property var scanAction
    property var cancelAction
    property var showOptionsAction
    property var allOptionsAction
    property var reselectDevicesAction
    property var showAboutAction

    Menu {
        title: i18nc("menu category", "File")

        MenuItem {
            text: newDocAction.text
            @KIRIGAMI_ICON_NAME@: newDocAction.icon.name
            onTriggered: newDocAction.trigger()
            enabled: newDocAction.enabled
        }

        MenuItem {
            text: saveDocAction.text
            @KIRIGAMI_ICON_NAME@: saveDocAction.icon.name
            onTriggered: saveDocAction.trigger()
            enabled: saveDocAction.enabled
        }

        MenuItem {
            text: shareAction.text
            @KIRIGAMI_ICON_NAME@: shareAction.icon.name
            onTriggered: shareAction.trigger()
            enabled: shareAction.enabled
        }

        MenuItem {
            text: printAction.text
            @KIRIGAMI_ICON_NAME@: printAction.icon.name
            onTriggered: printAction.trigger()
            enabled: printAction.enabled
        }

        MenuSeparator {
        }

        MenuItem {
            text: quitAction.text
            @KIRIGAMI_ICON_NAME@: quitAction.icon.name
            onTriggered: quitAction.trigger()
            enabled: quitAction.enabled
        }
    }

    Menu {
        title: i18nc("menu category", "Scan")

        MenuItem {
            text: previewAction.text
            @KIRIGAMI_ICON_NAME@: previewAction.icon.name
            onTriggered: previewAction.trigger()
            enabled: previewAction.enabled
        }

        MenuItem {
            text: scanAction.text
            @KIRIGAMI_ICON_NAME@: scanAction.icon.name
            onTriggered: scanAction.trigger()
            enabled: scanAction.enabled
        }

        MenuItem {
            text: cancelAction.text
            @KIRIGAMI_ICON_NAME@: cancelAction.icon.name
            onTriggered: cancelAction.trigger()
            enabled: cancelAction.enabled
        }
        
        MenuSeparator {
        }

        MenuItem {
            text: showOptionsAction.text
            @KIRIGAMI_ICON_NAME@: showOptionsAction.icon.name
            onTriggered: showOptionsAction.trigger()
            enabled: showOptionsAction.enabled
            checkable: showOptionsAction.checkable
            checked: showOptionsAction.checked
        }
        
        MenuItem {
            text: allOptionsAction.text
            @KIRIGAMI_ICON_NAME@: allOptionsAction.icon.name
            onTriggered: allOptionsAction.trigger()
            enabled: allOptionsAction.enabled
            checkable: allOptionsAction.checkable
            checked: allOptionsAction.checked
        }

        MenuSeparator {
        }
        
        MenuItem {
            text: reselectDevicesAction.text
            @KIRIGAMI_ICON_NAME@: reselectDevicesAction.icon.name
            onTriggered: reselectDevicesAction.trigger()
            enabled: reselectDevicesAction.enabled
        }
    }

    Menu {
        title: i18nc("menu category", "Help")

        MenuItem {
            text: showAboutAction.text
            @KIRIGAMI_ICON_NAME@: showAboutAction.icon.name
            onTriggered: showAboutAction.trigger()
            enabled: showAboutAction.enabled
        }
    }
}
