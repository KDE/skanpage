/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import org.kde.kirigami 2.5 as Kirigami
import org.kde.skanpage 1.0

Window {
    id: optionsWindow

    title: i18n("Scanner Options")
    color: Kirigami.Theme.backgroundColor
    
    flags: Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint
           | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint

    minimumHeight: 400
    minimumWidth: 600
    
    ScrollView {
        id: optionsView

        anchors.horizontalCenter: parent.horizontalCenter
        leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

        Kirigami.FormLayout {
            id: form

            Repeater {
                model: skanpage.optionsModel

                delegate: RowLayout {
                    id: delegate
                    Kirigami.FormData.label: (model.type == KSaneOption.TypeBool || model.type == KSaneOption.TypeAction) ? "" : model.title + ":"

                    visible: model.visible && model.type != KSaneOption.TypeGamma && model.type != KSaneOption.TypeDetectFail

                    OptionDelegate {
                        modelItem: model
                        onValueChanged: model.value = value
                    }
                } 
            }
        }
    }
}

