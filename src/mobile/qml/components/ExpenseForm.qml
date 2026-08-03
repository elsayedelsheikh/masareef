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
    // True once anything has been typed or picked. The sheets use this to
    // decide whether closing left a draft worth resuming.
    readonly property bool hasContent: amountText.length > 0
        || descriptionText.length > 0 || notesText.length > 0
        || selectedCategoryId > 0

    function clear() {
        amountText = ""
        descriptionText = ""
        notesText = ""
        selectedCategoryId = -1
        date = new Date()
    }

    // Puts the cursor in the first field, so opening the sheet and typing
    // just works instead of needing a tap first.
    function focusFirstField() {
        amountField.forceActiveFocus()
    }

    spacing: Theme.spacingM

    AmountField {
        id: amountField
        Layout.fillWidth: true
    }

    FieldLabel { text: qsTr("Category") }

    CategoryPicker {
        id: categoryPicker
        Layout.fillWidth: true
        model: form.categories
    }

    FieldLabel { text: qsTr("Date") }

    RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spacingS

        DateField {
            id: dateField
            Layout.fillWidth: true
        }

        // Most expenses are logged the same day or the day after; these save
        // a trip through the calendar popup.
        Button {
            flat: true
            text: qsTr("Today")
            implicitHeight: Theme.touchTarget
            enabled: !dateField.isToday
            onClicked: dateField.value = new Date()
        }
    }

    TextField {
        id: descriptionField
        Layout.fillWidth: true
        placeholderText: qsTr("Description")
        implicitHeight: Theme.fieldHeight
        // "Next" on the software keyboard moves on instead of dismissing it
        EnterKey.type: Qt.EnterKeyNext
        onAccepted: notesField.forceActiveFocus()
    }

    TextField {
        id: notesField
        Layout.fillWidth: true
        placeholderText: qsTr("Notes (optional)")
        implicitHeight: Theme.fieldHeight
        EnterKey.type: Qt.EnterKeyDone
    }
}
