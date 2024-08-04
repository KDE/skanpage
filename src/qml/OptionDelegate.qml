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

Item {
    id: optionDelegate

    property var modelItem
    property bool editMode: false
    
    implicitHeight: column.implicitHeight
    implicitWidth: column.implicitWidth
    
    Loader {
        active: editMode
        visible: active

        anchors.fill: parent

        sourceComponent:  Rectangle {
            color: model.quickAccess || mouseArea.hovered ? Kirigami.Theme.highlightColor : 'transparent'

            Kirigami.Separator {
                anchors.bottom: parent.bottom
                width: parent.width
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: model.quickAccess = !model.quickAccess
            }
        }
    }

    Column {
        id: column
        anchors.fill: parent

        spacing: Kirigami.Units.smallSpacing
        padding: Kirigami.Units.smallSpacing
      
        Label {
            visible: modelItem.type !== KSaneOption.TypeBool && modelItem.type !== KSaneOption.TypeGamma
            text: i18n("%1:", model.title)
        }

        RowLayout {
            enabled: !editMode

            Loader {
                active: modelItem.type === KSaneOption.TypeInteger
                visible: active

                sourceComponent: IntegerSpinBoxWithSuffix {
                    id: integerSpinBox

                    stepSize: modelItem.step
                    from: modelItem.minimum
                    to: modelItem.maximum
                    suffix: getUnitString(modelItem.unit)
                    value: modelItem.value
                    editable: true

                    onValueModified: function(value) {
                        if (value !== modelItem.value) {
                            modelItem.value = value
                        }
                    }
                }
            }

            Loader {
                active: modelItem.type === KSaneOption.TypeDouble
                visible: active

                sourceComponent: DoubleSpinBoxWithSuffix {
                    id: doubleSpinBox

                    stepSize: modelItem.step
                    from: modelItem.minimum
                    to: modelItem.maximum

                    suffix: getUnitString(modelItem.unit)
                    Binding on value {value: modelItem.value}
                    editable: true

                    onValueModified: function(value) {
                        if (!doubleSpinBox.approxeq(value, modelItem.value, stepSize / 1000)) {
                            modelItem.value = value
                        }
                    }
                }
            }

            Loader {
                active: modelItem.type === KSaneOption.TypeString
                visible: active

                sourceComponent: TextField {
                    text: modelItem.value
                    onTextChanged: {
                        if (text !== modelItem.value) {
                            modelItem.value = value
                        }
                    }
                }
            }

            Loader {
                active: modelItem.type === KSaneOption.TypeBool
                visible: active

                sourceComponent: CheckBox {
                    text: modelItem.title
                    checked: modelItem.value
                    onClicked: modelItem.value = !modelItem.value
                }
            }

            Loader {
                active: modelItem.type === KSaneOption.TypeAction
                visible: active

                sourceComponent: Button {
                    text: modelItem.title
                    onClicked: modelItem.value = 1
                }
            }

            Loader {
                active: modelItem.type === KSaneOption.TypeGamma
                visible: active

                sourceComponent: ColumnLayout {
                    readonly property bool isDefaultConfig:
                        modelItem.value[0] === 0 && modelItem.value[1] === 0 && modelItem.value[2] === 100
                    RowLayout {
                        Label {
                            text: i18nc("Add ':' to make a header text", "%1:", model.title)
                        }
                        CheckBox {
                            id: gammaEnableCheckbox
                            text: i18nc("@option:check a noun, as in 'the default setting'", "Default")
                            checked: isDefaultConfig
                            onClicked: {
                                if (checked) modelItem.value = [0, 0, 100]
                                else checked = false // Remove binding
                            }
                        }
                    }
                    Frame {
                        visible: !gammaEnableCheckbox.checked
                        ColumnLayout {
                            property int tmpVal: 0 // KSaneCore accepts int, Slider gives floats
                            spacing: Kirigami.Units.smallSpacing
                            Label {
                                text: i18nc("%1 is a numeric value", "Brightness: %1", modelItem.value[0])
                            }
                            Slider {
                                implicitWidth: 10 * Kirigami.Units.gridUnit
                                from: -50; to: 50
                                value: modelItem.value[0]
                                onMoved: {
                                    gammaEnableCheckbox.checked = false // Remove binding
                                    parent.tmpVal = value
                                    modelItem.value = [parent.tmpVal, modelItem.value[1], modelItem.value[2]]
                                }
                            }
                            Label {
                                text: i18nc("%1 is a numeric value", "Contrast: %1", modelItem.value[1])
                            }
                            Slider {
                                implicitWidth: 10 * Kirigami.Units.gridUnit
                                from: -50; to: 50
                                value: modelItem.value[1]
                                onMoved: {
                                    gammaEnableCheckbox.checked = false // Remove binding
                                    parent.tmpVal = value
                                    modelItem.value = [modelItem.value[0], parent.tmpVal, modelItem.value[2]]
                                }
                            }
                            Label {
                                text: i18nc("%1 is a numeric value", "Gamma: %1", modelItem.value[2])
                            }
                            Slider {
                                implicitWidth: 10 * Kirigami.Units.gridUnit
                                from: 30; to: 300
                                value: modelItem.value[2]
                                onMoved: {
                                    gammaEnableCheckbox.checked = false // Remove binding
                                    parent.tmpVal = value
                                    modelItem.value = [modelItem.value[0], modelItem.value[1], parent.tmpVal]
                                }
                            }
                        }
                    }
                }
            }

            Loader {
                id: comboLoader
                active: modelItem.type === KSaneOption.TypeValueList
                visible: active

                property var entries: modelItem.valueList
                property var modelValue: modelItem.value
                property string unitSuffix: getUnitString(modelItem.unit)

                sourceComponent: ComboBox {
                    id: combo

                    model: entries
                    displayText: unitSuffix === "" || currentText === "" ? currentText : i18nc("Adding unit suffix","%1 %2", currentText, unitSuffix)
                    currentIndex: indexOfValue(modelItem.value)

                    onCurrentValueChanged: {
                        if (combo.currentValue !== modelItem.value) {
                            modelItem.value = combo.currentValue
                        }
                    }

                    Connections {
                        target: comboLoader
                        function onModelValueChanged() {
                            currentIndex = indexOfValue(modelItem.value)
                        }
                    }

                    Component.onCompleted: currentIndex = indexOfValue(modelItem.value)
                }
            }
        }
    }

    function getUnitString(unitType) {
        switch (unitType) {
            case KSaneOption.UnitBit:
                return i18nc("Unit suffix for bit", "bit");
            case KSaneOption.UnitDPI:
                return i18nc("Unit suffix for DPI", "DPI");
            case KSaneOption.UnitMicroSecond:
                return i18nc("Unit suffix for microsecond", "µs");
            case KSaneOption.UnitSecond:
                return i18nc("Unit suffix for second", "s");
            case KSaneOption.UnitMilliMeter:
                return i18nc("Unit suffix for millimeter", "mm");
            case KSaneOption.UnitPercent:
                return i18nc("Unit suffix for percent", "%");
            case KSaneOption.UnitPixel:
                return i18nc("Unit suffix for pixel", "px");
            default:
                return "";
        }
    }
}
