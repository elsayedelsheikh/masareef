import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// The add/edit recurring-bill fields, shared by both bill sheets.
ColumnLayout {
    id: form

    property alias nameText: nameField.text
    property alias amountText: amountField.text
    property alias selectedCategoryId: categoryPicker.selectedCategoryId
    property alias nextDueDate: dateField.value
    // Index into the Recurrence enum (Monthly = 0, Quarterly = 1, Yearly = 2)
    property alias selectedRecurrence: recurrencePicker.currentIndex
    property alias notesText: notesField.text
    property var categories // CategoryListModel

    readonly property bool valid: amountField.valid
        && nameText.trim().length > 0
        && selectedCategoryId > 0
        && !isNaN(nextDueDate.getTime())

    readonly property bool hasContent: nameText.length > 0
        || amountText.length > 0 || notesText.length > 0
        || selectedCategoryId > 0

    function clear() {
        nameText = ""
        amountText = ""
        notesText = ""
        selectedCategoryId = -1
        selectedRecurrence = 0 // Monthly
        nextDueDate = new Date()
    }

    spacing: Theme.spacingM

    TextField {
        id: nameField
        Layout.fillWidth: true
        placeholderText: qsTr("Bill name")
        implicitHeight: Theme.fieldHeight
        EnterKey.type: Qt.EnterKeyNext
        onAccepted: amountField.forceActiveFocus()
    }

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

    FieldLabel { text: qsTr("Repeats") }

    SegmentedControl {
        id: recurrencePicker
        Layout.fillWidth: true
        options: [qsTr("Monthly"), qsTr("Quarterly"), qsTr("Yearly")]
    }

    FieldLabel { text: qsTr("Next due") }

    DateField {
        id: dateField
        Layout.fillWidth: true
    }

    TextField {
        id: notesField
        Layout.fillWidth: true
        placeholderText: qsTr("Notes (optional)")
        implicitHeight: Theme.fieldHeight
        EnterKey.type: Qt.EnterKeyDone
    }
}
