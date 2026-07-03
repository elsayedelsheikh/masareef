import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// The add/edit expense fields, shared by both sheets.
ColumnLayout {
    id: form

    property alias amountText: amountField.text
    property alias amountInputHints: amountField.inputMethodHints
    property alias descriptionText: descriptionField.text
    property alias notesText: notesField.text
    property alias selectedCategoryId: categoryPicker.selectedCategoryId
    property alias date: dateField.value
    property var categories // CategoryListModel

    readonly property bool valid: amountField.valid && selectedCategoryId > 0

    function clear() {
        amountText = ""
        descriptionText = ""
        notesText = ""
        selectedCategoryId = -1
        date = new Date()
    }

    spacing: Theme.spacingM

    AmountField {
        id: amountField
        Layout.fillWidth: true
    }

    Text {
        text: qsTr("Category")
        font.pixelSize: Theme.fontSizeCaption
        color: Theme.mutedInk
    }

    CategoryPicker {
        id: categoryPicker
        Layout.fillWidth: true
        model: form.categories
    }

    DateField {
        id: dateField
        Layout.alignment: Qt.AlignLeading
    }

    TextField {
        id: descriptionField
        Layout.fillWidth: true
        placeholderText: qsTr("Description")
        implicitHeight: Theme.touchTarget
    }

    TextField {
        id: notesField
        Layout.fillWidth: true
        placeholderText: qsTr("Notes (optional)")
        implicitHeight: Theme.touchTarget
    }
}
