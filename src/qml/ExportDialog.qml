/**
 * SPDX-FileCopyrightText: 2015 by Kåre Särs <kare.sars@iki .fi>
 * SPDX-FileCopyrightText: 2021 by Alexander Stippich <a.stippich@gmx.net>
 * SPDX-FileCopyrightText: 2024 by Thomas Duckworth <tduck973564@gmail.com>
 * SPDX-FileCopyrightText: 2024 by Milena Cole <mkoul@mail.ru>
 *  
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs

import org.kde.kirigami as Kirigami

import org.kde.skanpage

Kirigami.Dialog {
    id: exportDialog
    standardButtons: Kirigami.Dialog.Save | Kirigami.Dialog.Cancel

    implicitWidth: Kirigami.Units.gridUnit * 40

    property bool ocrChecked: false
    property string fileTitle
    property string fileName

    header: ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        Layout.margins: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        Kirigami.Heading {
            Layout.margins: Kirigami.Units.largeSpacing
            text: i18nc("@title:window", "Export PDF")
            level: 1
        }

        Kirigami.FormLayout {
            implicitWidth: Kirigami.Units.gridUnit * 30

            TextField {
                Kirigami.FormData.label: i18nc("@label", "Title:")
                text: skanpage.documentModel.name !== i18n("New document") ? skanpage.documentModel.name : skanpage.nameTemplate.parseSamples(skanpage.configuration.nameTemplate)
                onTextChanged: fileTitle = text
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.fillWidth: true
                Kirigami.FormData.label: i18nc("@label", "File:")

                TextField {
                    id: fileNameItem
                    Layout.fillWidth: true
                    text: skanpage.configuration.defaultFolder + "/" + fileTitle + ".pdf"
                    onTextChanged: fileName = text
                }

                Button {
                    icon.name: "document-open-symbolic"
                    text: i18nc("@action:button", "Save As…")
                    display: AbstractButton.IconOnly
                    ToolTip.text: text
                    ToolTip.delay: Kirigami.Units.toolTipDelay
                    ToolTip.visible: hovered
                    onClicked: fileNameDialog.open()
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: skanpage.languageModel.rowCount() > 0
        }
    }

    contentItem: ListView {
        id: listView
        implicitWidth: exportDialog.implicitWidth
        implicitHeight: visible ? Kirigami.Units.gridUnit * 25 : 0
        clip: true

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
        }

        headerPositioning: ListView.OverlayHeader
        header: Kirigami.InlineViewHeader {
            width: scrollBar.visible ? listView.width - scrollBar.width : listView.width
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing

            text: i18n("Optical Character Recognition")

            Switch {
                text: i18nc("@option:check Enable optical character recognition", "Enable")
                enabled: skanpage.languageModel.rowCount() > 0
                checked: ocrChecked
                onCheckedChanged: ocrChecked = checked
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:usagetip", "If the required languages are not listed, install Tesseract's language file with the system's package manager.")
            }
        }

        footerPositioning: ListView.OverlayFooter
        footer: Kirigami.Separator {
            width: listView.width
        }

        model: skanpage.languageModel
        delegate: CheckDelegate {
            enabled: ocrChecked
            implicitWidth: scrollBar.visible ? listView.width - scrollBar.width : listView.width
            text: "%1 [%2]".arg(model.name).arg(model.code)
            onClicked: model.use = checked
        }
    }

    FileDialog {
        id: fileNameDialog
        currentFolder: skanpage.configuration.defaultFolder
        fileMode: FileDialog.SaveFile
        nameFilters: skanpage.formatModel.pdfFormatFilter()
        onAccepted: fileNameItem.text = selectedFile
    }

    onAccepted: {
        skanpage.documentModel.exportPDF(fileName, fileTitle, ocrChecked)
        exportDialog.close()
    }
}
