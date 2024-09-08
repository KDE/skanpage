/**
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigami.delegates as KD
import org.kde.purpose as Purpose
import org.kde.skanpage

Kirigami.Dialog {
    id: shareDialog

    title: i18n("Share")

    padding: Kirigami.Units.largeSpacing
    preferredWidth: Kirigami.Units.gridUnit * 25

    property int selectedIndex: -1

    signal error(var errorText)

    Purpose.PurposeAlternativesModel {
        id: alternativesModel
        inputData: {
            'urls': [],
            'mimeType': ["application/pdf"]
        }
        pluginType: "Export"
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            Label {
                text: i18n("Share as:")
            }

            ComboBox {
                id: formatSelection

                property string currentSuffix
                model: skanpage.formatModel
                textRole: "comment"
                valueRole: "name"
                onCurrentValueChanged: {
                    currentSuffix = skanpage.formatModel.getData(formatSelection.currentIndex, FormatModel.SuffixRole)
                    alternativesModel.inputData = {
                        'urls': [],
                        'mimeType': [currentValue]
                    }
                    purposeView.reset()
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        StackView {
            id: purposeView
            focus: true

            implicitHeight: alternativesList.contentItem.contentHeight

            Layout.fillWidth: true
            Layout.fillHeight: true

            function reset() {
                while(purposeView.depth > 1) {
                    purposeView.pop()
                }
            }

            function proceed(index) {
                purposeView.push(waitingComponent)
                skanpage.documentModel.createSharingFile(formatSelection.currentSuffix, [])
                shareDialog.selectedIndex = index
            }

            initialItem: ScrollView {
                id: alternativesList

                contentItem: ListView {

                    focus: true
                    model: alternativesModel

                    implicitHeight: contentHeight

                    currentIndex: -1

                    delegate: ItemDelegate {
                        id: shareDelegate

                        width: purposeView.width

                        required property string iconName
                        required property string actionDisplay
                        required property int index

                        contentItem: RowLayout {
                            spacing: Kirigami.Units.smallSpacing

                            KD.IconTitleSubtitle {
                                title: shareDelegate.actionDisplay
                                icon.name: shareDelegate.iconName
                                Layout.fillWidth: true
                            }
                            Kirigami.Icon {
                                implicitHeight: Kirigami.Units.iconSizes.smallMedium
                                implicitWidth: Kirigami.Units.iconSizes.smallMedium
                                source: "arrow-right"
                            }
                        }

                        onClicked: purposeView.proceed(shareDelegate.index)
                        Keys.onReturnPressed: purposeView.proceed(shareDelegate.index)
                        Keys.onEnterPressed: purposeView.proceed(shareDelegate.index)

                    }
                }
            }
        }

        Connections {
            target: skanpage.documentModel

            function onSharingDocumentsCreated(filePaths) {
                alternativesModel.inputData = {
                    'urls': filePaths,
                    'mimeType': [formatSelection.currentValue]
                }
                //remove any busy indicator in case there is one
                if (purposeView.depth === 2) {
                    purposeView.pop()
                }
                purposeView.push(jobComponent, {index: selectedIndex})
            }
        }
    }

    Component {
        id: jobComponent

        Purpose.JobView {
            id: jobView
            model: alternativesModel

            onStateChanged: {
                if (state === Purpose.PurposeJobController.Finished) {
                    shareDialog.close()
                } else if (state === Purpose.PurposeJobController.Error) {
                    shareDialog.close()
                    shareDialog.error(jobView.job.errorString)
                } else if (state === Purpose.PurposeJobController.Cancelled) {
                    purposeView.pop();
                }
            }

            Component.onCompleted: start()
        }
    }

    Component {
        id: waitingComponent

        Item {
            BusyIndicator {
                running: true
                anchors.centerIn: parent
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            purposeView.reset()
        }
    }
}
