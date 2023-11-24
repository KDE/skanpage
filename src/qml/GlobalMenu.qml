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
            icon.name: newDocAction.icon.name
            onTriggered: newDocAction.trigger()
            enabled: newDocAction.enabled
        }

        MenuItem {
            text: saveDocAction.text
            icon.name: saveDocAction.icon.name
            onTriggered: saveDocAction.trigger()
            enabled: saveDocAction.enabled
        }

        MenuItem {
            text: shareAction.text
            icon.name: shareAction.icon.name
            onTriggered: shareAction.trigger()
            enabled: shareAction.enabled
        }

        MenuItem {
            text: printAction.text
            icon.name: printAction.icon.name
            onTriggered: printAction.trigger()
            enabled: printAction.enabled
        }

        MenuSeparator {
        }

        MenuItem {
            text: quitAction.text
            icon.name: quitAction.icon.name
            onTriggered: quitAction.trigger()
            enabled: quitAction.enabled
        }
    }

    Menu {
        title: i18nc("menu category", "Scan")

        MenuItem {
            text: previewAction.text
            icon.name: previewAction.icon.name
            onTriggered: previewAction.trigger()
            enabled: previewAction.enabled
        }

        MenuItem {
            text: scanAction.text
            icon.name: scanAction.icon.name
            onTriggered: scanAction.trigger()
            enabled: scanAction.enabled
        }

        MenuItem {
            text: cancelAction.text
            icon.name: cancelAction.icon.name
            onTriggered: cancelAction.trigger()
            enabled: cancelAction.enabled
        }
        
        MenuSeparator {
        }

        MenuItem {
            text: showOptionsAction.text
            icon.name: showOptionsAction.icon.name
            onTriggered: showOptionsAction.trigger()
            enabled: showOptionsAction.enabled
            checkable: showOptionsAction.checkable
            checked: showOptionsAction.checked
        }
        
        MenuItem {
            text: allOptionsAction.text
            icon.name: allOptionsAction.icon.name
            onTriggered: allOptionsAction.trigger()
            enabled: allOptionsAction.enabled
            checkable: allOptionsAction.checkable
            checked: allOptionsAction.checked
        }

        MenuSeparator {
        }
        
        MenuItem {
            text: reselectDevicesAction.text
            icon.name: reselectDevicesAction.icon.name
            onTriggered: reselectDevicesAction.trigger()
            enabled: reselectDevicesAction.enabled
        }
    }

    Menu {
        title: i18nc("menu category", "Help")

        MenuItem {
            text: showAboutAction.text
            icon.name: showAboutAction.icon.name
            onTriggered: showAboutAction.trigger()
            enabled: showAboutAction.enabled
        }
    }
}
