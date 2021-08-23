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
    property var value
    property alias from: control.from
    property alias to: control.to
    property alias stepSize: control.stepSize 
    property alias suffix: suffixText.text
    property alias editable: control.editable
    
    implicitWidth: control.width
    implicitHeight: control.implicitHeight
    
    signal valueModified(var value)
    
    Connections {            
        target: container
        function onValueChanged() {
            control.value = container.value
        }
    }
    
    SpinBox {
        id: control

        value: container.value
        
        onValueChanged: {
            if (container.value != value) {
                container.valueModified(value)
            }
        }
                    
        TextMetrics {
            id: minTextSize 
            text: Number(control.from).toLocaleString(control.locale, 'f', 0)
        }
        
        TextMetrics {
            id: maxTextSize 
            text: Number(control.to).toLocaleString(control.locale, 'f', 0)
        }
        
        contentItem: Row {
            
            spacing: Kirigami.Units.smallSpacing
            leftPadding: Kirigami.Units.smallSpacing
            
            TextInput {
                id: textInput
                
                width: Math.max(minTextSize.width, maxTextSize.width)
                color: Kirigami.Theme.textColor

                selectByMouse: true

                text: control.textFromValue(control.value, control.locale)
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter

                readOnly: !control.editable
                validator: control.validator
                inputMethodHints: Qt.ImhFormattedNumbersOnly
 
                onTextEdited: {
                    var newValue = control.valueFromText(textInput.text.replace(/\D/g, ""), control.locale)
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

                font: textInput.font
                color: textInput.color
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                
                visible: text != ''
            }
        }
    }
}
