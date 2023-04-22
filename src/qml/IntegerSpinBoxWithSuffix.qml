/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami

Item {
    id: container
    property alias value: control.value
    property alias from: control.from
    property alias to: control.to
    property alias stepSize: control.stepSize
    property alias suffix: suffixText.text
    property alias editable: control.editable

    implicitWidth: control.width
    implicitHeight: control.implicitHeight

    signal valueModified(int value)

    SpinBox {
        id: control

        onValueChanged: {
            container.valueModified(value)
        }

        TextMetrics {
            id: minTextSize
            font: textInput.font
            text: control.textFromValue(control.from, control.locale)
        }

        TextMetrics {
            id: maxTextSize
            font: textInput.font
            text: control.textFromValue(control.to, control.locale)
        }

        contentItem: Row {
            spacing: Kirigami.Units.smallSpacing

            TextInput {
                id: textInput
                color: Kirigami.Theme.textColor
                readOnly: !control.editable
                text: control.textFromValue(control.value, control.locale)

                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                validator: control.validator
                inputMethodHints: Qt.ImhFormattedNumbersOnly

                onTextEdited: {
                    control.value = control.valueFromText(text.replace(control.locale.groupSeparator, ""), control.locale)
                    control.valueModified()
                }

                width: Math.max(minTextSize.width, maxTextSize.width) + Kirigami.Units.smallSpacing
            }

            Label {
                id: suffixText
                visible: text != ''
            }
        }
    }
}
