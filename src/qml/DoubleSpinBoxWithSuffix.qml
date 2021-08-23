/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.12 as Kirigami

/* This is way too complicated as it should be and more a giant hack.
 * Should be implemented properly in the future */

Item {
    id: container
    property var value 
    property real from
    property real to
    property real stepSize
    property alias suffix: suffixText.text
    property alias editable: control.editable
    
    signal valueModified(var value)
    
    implicitWidth: control.implicitWidth
    implicitHeight: control.implicitHeight

    Connections {            
        target: container
        function onValueChanged() {
            if (control.value != (container.value * control.multiplier)) {
                control.value = container.value * control.multiplier
            }
        }
    }
    
    SpinBox {
        id: control
        
        property int multiplier: 1
        property int decimals: 0
        property bool setup: true

        onValueChanged: {
            if (container.value != value && !setup) {
                container.valueModified(value / control.multiplier)
            }
        }

        textFromValue: function(value, locale) {
            return Number(value / control.multiplier).toLocaleString(locale, 'f', decimals)
        }

        valueFromText: function(text, locale) {
            return Number.fromLocaleString(locale, text.replace(locale.groupSeparator, "")) * control.multiplier
        }
        
        Component.onCompleted: {
            var step = container.stepSize
            control.decimals = 0
            control.multiplier = 1
            while (step < 1 && step != 0) {
                step = step * 10
                control.decimals = control.decimals + 1
                control.multiplier = control.multiplier * 10
            }
            control.to = container.to * control.multiplier
            control.from = container.from * control.multiplier
            control.stepSize = container.stepSize * control.multiplier
            control.value = container.value * control.multiplier
            setup = false
            minTextSize.text = Number(container.from).toLocaleString(control.locale, 'f', decimals)
            maxTextSize.text = Number(container.to).toLocaleString(control.locale, 'f', decimals)
            textInput.text = Qt.binding(function() { return control.textFromValue(control.value, control.locale) }) 
        }
   
        TextMetrics {
            id: minTextSize 
        }
                    
        TextMetrics {
            id: maxTextSize 
        }

        contentItem: Row {
            
            spacing: Kirigami.Units.smallSpacing
            leftPadding: Kirigami.Units.smallSpacing
            
            TextInput {
                id: textInput
                selectByMouse: true

                color: Kirigami.Theme.textColor
                width: Math.max(minTextSize.width, maxTextSize.width)
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter

                readOnly: !control.editable
                
                onTextChanged: {
                    var newValue = control.valueFromText(textInput.text, control.locale)
                    if (!isFinite(newValue)) {
                        return
                    }
                    if (newValue > control.to) {
                        newValue = control.to
                    }
                    if (newValue < control.from) {
                        newValue = control.from                                          
                    }
                    if (control.value != newValue) {
                        control.value = newValue
                    }
                } 
            }
            
            Text {
                id: suffixText
                z: -1
                font: textInput.font
                color: textInput.color
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
