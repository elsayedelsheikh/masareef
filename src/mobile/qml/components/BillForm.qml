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
    // The segmented control's index. It is not a Recurrence value: the two
    // orders agree today, but the segments are a UI choice and the enum is
    // a storage one, so the sheets convert through recurrenceFor() /
    // segmentFor() below rather than passing the index through.
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

    // The one place the segment order and the Recurrence enum are tied
    // together: the segments below are built from it, and both bill sheets
    // convert through the two functions, so reordering either list cannot
    // quietly save the wrong cadence.
    readonly property var recurrenceOrder: [Recurrence.Monthly,
                                            Recurrence.Quarterly,
                                            Recurrence.Yearly]

    function recurrenceFor(segmentIndex) {
        return segmentIndex >= 0 && segmentIndex < recurrenceOrder.length
            ? recurrenceOrder[segmentIndex]
            : Recurrence.Monthly
    }

    function segmentFor(recurrence) {
        const index = recurrenceOrder.indexOf(recurrence)
        return index >= 0 ? index : 0
    }

    function clear() {
        nameText = ""
        amountText = ""
        notesText = ""
        selectedCategoryId = -1
        selectedRecurrence = form.segmentFor(Recurrence.Monthly)
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
        // One label per entry of recurrenceOrder, in the same order.
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
