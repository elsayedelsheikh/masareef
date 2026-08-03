import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for adding an expense.
//
// The form is deliberately *not* cleared when the sheet closes. Only Cancel
// and a successful save clear it, so an interruption — the back button, a
// mis-tap, switching apps — leaves the half-filled entry intact and
// reopening resumes it instead of starting over.
BottomSheet {
    id: sheet

    property ExpenseController controller

    property alias amountText: form.amountText
    property alias amountInputHints: form.amountInputHints
    property alias descriptionText: form.descriptionText
    property alias notesText: form.notesText
    property alias selectedCategoryId: form.selectedCategoryId
    property alias date: form.date
    readonly property bool canSave: form.valid
    // True while the sheet is showing an entry started before it was closed
    property bool resumedDraft: false
    // True while the sheet is showing an entry filled in from somewhere else
    property bool prefilled: false

    property string errorMessage: ""

    // Opens the sheet already filled in from another screen — logging a
    // price book item as an expense. This replaces whatever draft was in
    // the form, so the sheet says so rather than silently swapping it.
    // The parameters are named apart from the sheet's own amountText and
    // descriptionText properties, which they would otherwise shadow.
    function openPrefilled(categoryId, amount, text) {
        startOver()
        form.selectedCategoryId = categoryId
        form.amountText = amount
        form.descriptionText = text
        form.date = new Date()
        prefilled = true
        open()
    }

    function save() {
        if (!canSave || !controller)
            return
        const id = controller.add(form.selectedCategoryId, form.amountText,
                                  form.descriptionText, form.date, form.notesText)
        if (id > 0) {
            startOver()
            close()
        } else {
            // Nothing was saved, so the form keeps everything it had.
            errorMessage = controller.lastError
        }
    }

    function startOver() {
        form.clear()
        resumedDraft = false
        prefilled = false
        errorMessage = ""
    }

    function discard() {
        startOver()
        close()
    }

    heading: qsTr("Add expense")
    acceptEnabled: canSave

    onAccepted: save()
    onCancelled: discard()

    onAboutToShow: {
        categoriesModel.refresh()
        errorMessage = ""
        // A prefilled sheet was not "left off" anywhere; it says so itself.
        resumedDraft = !prefilled && form.hasContent
    }

    CategoryListModel {
        id: categoriesModel
    }

    InlineBanner {
        Layout.fillWidth: true
        visible: sheet.resumedDraft
        message: qsTr("Picked up where you left off.")
        actionText: qsTr("Start over")
        onActionTriggered: sheet.startOver()
    }

    InlineBanner {
        Layout.fillWidth: true
        visible: sheet.prefilled
        message: qsTr("Filled in from the price book.")
        actionText: qsTr("Clear")
        onActionTriggered: sheet.startOver()
    }

    InlineBanner {
        Layout.fillWidth: true
        visible: sheet.errorMessage.length > 0
        message: sheet.errorMessage
        tint: Theme.criticalTint
        ink: Theme.critical
    }

    ExpenseForm {
        id: form
        Layout.fillWidth: true
        categories: categoriesModel
    }
}
