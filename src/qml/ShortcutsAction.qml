/**
 * SPDX-FileCopyrightText: 2023 by John Doe <kde.sp90x@simplelogin.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15

import org.kde.kirigami as Kirigami
import org.kde.skanpage 1.0

/* Tiny, tiny, utility for making an action's shortcut configurable */

Kirigami.Action {
    id: _action
    property string shortcutsName
    property var _shortcuts_var: Shortcut { id: _shortcuts; onActivated: _action.trigger() }
    Component.onCompleted: { skanpage.registerAction(_action, _shortcuts, icon.name) }
}
