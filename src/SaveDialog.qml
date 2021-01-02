/* ============================================================
 *
 * Copyright (C) 2015-2016 by Kåre Särs <kare.sars@iki .fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * ============================================================ */
import QtQuick 2.7
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import org.kde.skanpage 1.0

Window {
    id: saveOptions

    property variant doc

    property alias fileName: fileNameItem.text
    title: i18n("Document Properties")
    
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.CustomizeWindowHint | Qt.WindowTitleHint
           | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint
           
    SystemPalette { id: palette }
    
    color: palette.window

    minimumHeight: 200
    minimumWidth: 450

    GridLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 4
        columns: 3

        Text {
            text: i18n("Title:"); Layout.alignment: Qt.AlignRight
        }
        
        TextField {
            id: fileTitleItem
            Layout.fillWidth: true
            Layout.columnSpan: 1
            onEditingFinished: {
                // After explicitly editing the title disable the binding
                var tmp = text;
                text = tmp
            }
        }
        
        Item {
            width: 1
            height: 1
        }

        Text {
            text: i18n("Page Size:")
            Layout.alignment: Qt.AlignRight
        }
        
        ComboBox {
            id: sizeCombo
            Layout.columnSpan: 2
            model: skanPage.scanSizes
            Layout.alignment: Qt.AlignLeft
        }

        Text {
            text: i18n("Print Quality:")
            Layout.alignment: Qt.AlignRight
        }
        
        ComboBox {
            id: resCombo
            Layout.columnSpan: 2
            model: resolutions
            currentIndex: 1
            textRole: "name"
            Layout.alignment: Qt.AlignLeft
        }

        Text { 
            text: i18n("File:")
            Layout.alignment: Qt.AlignRight
        }

        TextField {
            id: fileNameItem
            Layout.fillWidth: true
            Keys.onReturnPressed: trySave();
        }

        ToolButton {
            text: "..."
            width: height
            onClicked: {
                fileDialog.fileUrl = fileNameItem.text
                fileDialog.open()
            }
        }
        
        Item { 
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.columnSpan: 3
        }
    }

    function trySave() {
        //console.log("save clicked", fileNameItem.text, pages.fileExists(fileNameItem.text))
        if (pages.fileExists(fileNameItem.text) && (fileDialog.lastSelected != fileNameItem.text)) {
            overwrite.file = fileDialog.toFileName(fileNameItem.text)
            overwrite.visible = true;
        }
        else {
            saveFile();
            saveOptions.visible = false
        }
    }

    RowLayout {
        id: buttonRow
        
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: 4
        }
        
        Button {
            id: saveButton
            icon.name: "document-save"
            text: i18n("Save")
            onClicked: trySave();
        }
        
        Button {
            action: cancelAction
        }
    }
    
    Action {
        id: cancelAction
        icon.name: "dialog-close"
        text: i18n("Cancel")
        shortcut: "Esc"
        onTriggered: saveOptions.visible = false
    }

    ListModel {
        id: resolutions
        
        Component.onCompleted: {
            resolutions.append({ name: i18n("Draft (75 DPI)"), resolution: 75 });
            resolutions.append({ name: i18n("Normal (150 DPI)"), resolution: 150 });
            resolutions.append({ name: i18n("High Quality (300 DPI)"), resolution: 300 });
            resolutions.append({ name: i18n("Best Quality (600 DPI)"), resolution: 600 });
        }
    }

    SaveFileDialog {
        id: fileDialog
        title: i18n("Save as ...")
        nameFilters: [ i18n("PDF documents (*.pdf)") ]
        property string lastSelected: ""
        onAccepted: {
            fileNameItem.text = doc.toDisplayString(fileDialog.fileUrl)
            lastSelected = fileNameItem.text
        }
    }

    function saveFile() {
        if (typeof doc === "object" && typeof doc.save === "function") {
            //console.log(skanPage.scanSizesF, skanPage.scanSizesF[sizeCombo.currentIndex], sizeCombo.currentIndex)
            //console.log(skanPage.scanSizesF.length)
            doc.save(fileNameItem.text,
                     skanPage.scanSizesF[sizeCombo.currentIndex],
                     resolutions.get(resCombo.currentIndex).resolution,
                     fileTitleItem.text)
        }
    }

    Dialog {
        id: overwrite
        property string file: ""
        title: i18n("Overwrite File?")
        modal: true 

        standardButtons: Dialog.Yes | Dialog.No
        onAccepted: {
            saveFile();
            saveOptions.visible = false;
            overwrite.visible = false;
        }
        onRejected: {
            overwrite.visible = false;
        }
        
        Label {
            text: i18n("\n\nThe file \"%1\" already exists. Do you want to overwrite it?").arg(overwrite.file)
        }
    }

    function open(fileName) {
        fileDialog.lastSelected = "";
        saveDialog.fileName = fileName === "" ? fileDialog.documentsDir() + "/" + i18n("Unnamed") + ".pdf" : fileName;
        saveDialog.visible = true;
        fileTitleItem.text =  Qt.binding(function() {return fileDialog.toBaseName(fileNameItem.text)});
    }
}
