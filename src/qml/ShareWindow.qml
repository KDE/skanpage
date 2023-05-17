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
import org.kde.purpose 1.0 as Purpose
import org.kde.skanpage 1.0

Window {
    id: shareWindow

    title: i18n("Share")
    color: Kirigami.Theme.backgroundColor

    flags: Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint
           | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint

    minimumHeight: 300
    minimumWidth: 300

    property int selectedIndex: -1

    signal error(var errorText)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

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

        Purpose.PurposeAlternativesModel {
            id: alternativesModel
            inputData: {
                'urls': [],
                'mimeType': ["application/pdf"]
            }
            pluginType: "Export"
        }

        StackView {
            id: purposeView
            focus: true

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
                shareWindow.selectedIndex = index
            }

            initialItem: ScrollView {
                id: alternativesList

                // FIXME: Remove once the Qt bug always showing the horizontal scrollbar is resolved
                ScrollBar.horizontal.visible: false

                contentItem: ListView {
                    focus: true
                    model: alternativesModel

                    implicitHeight: contentHeight

                    currentIndex: -1

                    delegate: Kirigami.BasicListItem {
                        id: shareDelegate

                        //required property string display
                        required property string iconName
                        required property int index

                        label: shareDelegate.display
                        icon: shareDelegate.iconName
                        onClicked: purposeView.proceed(shareDelegate.index)
                        Keys.onReturnPressed: purposeView.proceed(shareDelegate.index)
                        Keys.onEnterPressed: purposeView.proceed(shareDelegate.index)

                        trailing: Kirigami.Icon {
                            source: "arrow-right"
                        }
                    }
                }

                Component.onCompleted: {
                    alternativesList.background.visible = true;
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
                if (purposeView.depth == 2) {
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
                    shareWindow.close()
                } else if (state === Purpose.PurposeJobController.Error) {
                    shareWindow.close()
                    shareWindow.error(jobView.job.errorString)
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
