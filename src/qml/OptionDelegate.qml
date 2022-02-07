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
            visible: modelItem.type !== KSaneOption.TypeBool
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

                    onValueModified: {
                        if (value != modelItem.value) {
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
                    value: modelItem.value
                    editable: true

                    onValueModified: {
                        if (value != modelItem.value) {
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
                    onTextChanged:  {
                        if (text != modelItem.value) {
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
                        if (combo.currentValue != modelItem.value) {
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
                break;
            case KSaneOption.UnitDPI:
                return i18nc("Unit suffix for DPI", "DPI");
                break;
            case KSaneOption.UnitMicroSecond:
                return i18nc("Unit suffix for microsecond", "µs");
                break;
            case KSaneOption.UnitSecond:
                return i18nc("Unit suffix for second", "s");
                break;
            case KSaneOption.UnitMilliMeter:
                return i18nc("Unit suffix for millimeter", "mm");
                break;
            case KSaneOption.UnitPercent:
                return i18nc("Unit suffix for percent", "%");
                break;
            case KSaneOption.UnitPixel:
                return i18nc("Unit suffix for pixel", "px");
                break;
            default:
                return "";
        }
    }
}
