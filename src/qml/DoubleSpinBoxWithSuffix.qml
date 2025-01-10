/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.kirigami as Kirigami

/* This is way too complicated as it should be and more a giant hack.
 * Should be implemented properly in the future */

Item {
    id: container
    property real value
    property real from
    property real to
    property real stepSize
    property alias suffix: suffixText.text
    property alias editable: control.editable

    signal valueModified(real value)

    implicitWidth: control.width
    implicitHeight: control.implicitHeight

    onStepSizeChanged: control.findMultiplier() // Needed when reloading scanner
    property var approxeq: function(v1, v2, epsilon) { return Math.abs(v1 - v2) < epsilon }

    SpinBox {
        id: control

        property int multiplier: 1
        property int decimals: 0
        property bool setup: true

        from: container.from * multiplier
        to: container.to * multiplier
        stepSize: container.stepSize * multiplier
        Binding on value {value: Math.round(container.value * control.multiplier)}

        onValueModified: {
            if (!approxeq(container.value, value / multiplier, stepSize / 1000) && !setup) {
                container.value = value / multiplier
                container.valueModified(container.value)
            }
        }

        textFromValue: function(value, locale) {
            return Number(value / multiplier).toLocaleString(locale, 'f', decimals)
        }

        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text) * multiplier
        }

        Component.onCompleted: findMultiplier()
        function findMultiplier() {
            while (container.stepSize * multiplier < 1 && container.stepSize !== 0) {
                multiplier = multiplier * 10
                decimals = decimals + 1
            }
            setup = false
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
                visible: text !== ''
            }
        }
    }
}
