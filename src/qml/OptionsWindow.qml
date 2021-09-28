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

    onClosing: skanpage.optionsModel.resetOptionsValues()

    ColumnLayout {
        anchors.fill: parent

        ScrollView {
            id: optionsView

            Layout.fillHeight: true
            Layout.fillWidth: true

            leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

            enabled: skanpage.applicationState == Skanpage.ReadyForScan

            Kirigami.FormLayout {
                id: form

                width: optionsView.width
                height: children.height

                Repeater {
                    model: skanpage.optionsModel

                    delegate: RowLayout {
                        id: delegate

                        Kirigami.FormData.label: (model.type == KSaneOption.TypeBool || model.type == KSaneOption.TypeAction) ? "" : i18nc("Separator between a title and a value", "%1:", model.title)

                        visible: model.visible && model.type != KSaneOption.TypeGamma && model.type != KSaneOption.TypeDetectFail

                        OptionDelegate {
                            modelItem: model
                            onValueChanged: model.value = value
                        }
                    }
                }
            }
        }

        RowLayout {

            DialogButtonBox {

                Button {
                    text: i18n("Reset")
                    icon.name: 'edit-undo'
                    enabled: skanpage.optionsModel.isModified
                    DialogButtonBox.buttonRole: DialogButtonBox.ResetRole
                    onClicked: skanpage.optionsModel.resetOptionsValues()
                }

            }

            Item {
                Layout.fillWidth: true
            }

            DialogButtonBox {
                Button {
                    text: i18n("Apply")
                    icon.name: 'dialog-ok-apply'
                    enabled: skanpage.optionsModel.isModified
                    DialogButtonBox.buttonRole: DialogButtonBox.ApplyRole
                    onClicked: skanpage.optionsModel.saveOptionsValues()
                }

                Button {
                    text: i18n("Close")
                    icon.name: 'dialog-cancel'
                    DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                    onClicked: optionsWindow.close()
                }
            }
        }
    }
}

