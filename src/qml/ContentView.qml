/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import org.kde.kirigami as Kirigami
import org.kde.skanpage 1.0

Item {
    id: mainContent

    readonly property string name: skanpage.documentModel.changed ? i18nc("Prefix for document name indicating an unsaved document", "* %1", skanpage.documentModel.name) : skanpage.documentModel.name;
    property int splitViewPreferredWidth: splitView.width / 4
    property alias splitViewItemWidth: scrollView.width
    property bool showOptions: true
    property alias optionsPanel: optionsPanel

    signal saveSinglePage(int pageNumber)

    focus: true

    RowLayout {
        anchors.fill: parent
        
        spacing: 0

        SplitView {
            id: splitView
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            handle: Item {
                implicitWidth: Kirigami.Units.smallSpacing

                Kirigami.Separator {
                    height: parent.height
                }
            }

            DocumentList {
                id: scrollView

                SplitView.fillHeight: true
                SplitView.preferredWidth: splitViewPreferredWidth
                SplitView.minimumWidth: scrollView.minimumWidth
                SplitView.maximumWidth: splitView.width - activeDocument.width - optionsSeparator.width
                onSaveSinglePage: function (pageNumber) {
                    mainContent.saveSinglePage(pageNumber)
                }
                onMinimumWidthChanged: // Do not prefer a width lower than a minimum
                    if (SplitView.preferredWidth < minimumWidth) SplitView.preferredWidth = minimumWidth
            }

            onResizingChanged: if (!resizing) scrollView.minimumWidthChanged()

            DocumentPage {
                id: activeDocument

                SplitView.fillWidth: true
                SplitView.minimumWidth: splitView.width / 5
                SplitView.fillHeight: true

                visible: skanpage.applicationState === Skanpage.ReadyForScan
                onSaveSinglePage: function (pageNumber) {
                    mainContent.saveSinglePage(pageNumber)
                }
            }

            InProgressPage {
                id: inProgressImage

                SplitView.fillWidth: true
                SplitView.fillHeight: true

                visible: skanpage.applicationState === Skanpage.ScanInProgress
            }
        }

        Item {
            id: optionsSeparator
            Layout.fillHeight: true

            Kirigami.Separator {
                height: parent.height
            }
        }

        OptionsPanel {
            id: optionsPanel

            Layout.fillHeight: true
        }
    }

    states: [
        State {
            name: "noOptions"
            when: mainContent.showOptions === false
            PropertyChanges {
                target: optionsPanel
                Layout.minimumWidth: 0
                Layout.maximumWidth: 0
                Layout.preferredWidth: 0
            }
            PropertyChanges {
                target: optionsSeparator
                visible: false
            }
        },
        State {
            name: "showOptions"
            when: mainContent.showOptions === true
            PropertyChanges {
                target: optionsPanel
                Layout.minimumWidth: optionsPanel.targetWidth
                Layout.maximumWidth: optionsPanel.targetWidth
                Layout.preferredWidth: optionsPanel.targetWidth
            }
            PropertyChanges {
                target: optionsSeparator
                visible: true
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            target: optionsPanel
            properties: "Layout.minimumWidth, Layout.maximumWidth, Layout.preferredWidth"
            easing.type: Easing.InOutQuad
            duration: Kirigami.Units.longDuration
        }
    }
}
