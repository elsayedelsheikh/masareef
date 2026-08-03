import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for editing an existing expense. Call openFor(id).
BottomSheet {
    id: sheet

    property ExpenseController controller
    property int expenseId: 0

    property alias amountText: form.amountText
    property alias descriptionText: form.descriptionText
    property alias notesText: form.notesText
    property alias selectedCategoryId: form.selectedCategoryId
    property alias date: form.date
    readonly property bool canSave: form.valid

    property string errorMessage: ""

    function openFor(id) {
        if (!controller)
            return
        errorMessage = ""
        categoriesModel.refresh()
        // Load before committing to the id: if the expense is gone (deleted
        // on another screen) the sheet must not stay bound to it.
        if (!controller.load(id)) {
            expenseId = 0
            errorMessage = controller.lastError
            return
        }
        expenseId = id
        form.amountText = controller.editAmountText
        form.selectedCategoryId = controller.editCategoryId
        form.descriptionText = controller.editDescription
        form.notesText = controller.editNotes
        form.date = controller.editDate
        open()
    }

    function save() {
        if (!canSave || !controller || expenseId <= 0 || errorMessage.length > 0)
            return
        if (controller.update(expenseId, form.selectedCategoryId, form.amountText,
                              form.descriptionText, form.date, form.notesText))
            close()
        else
            errorMessage = controller.lastError
    }

    function removeExpense() {
        if (!controller || expenseId <= 0 || errorMessage.length > 0)
            return
        if (controller.remove(expenseId))
            close()
        else
            errorMessage = controller.lastError
    }

    heading: qsTr("Edit expense")
    acceptEnabled: canSave
    deletable: true

    onAccepted: save()
    onCancelled: close()
    onDeleteRequested: confirmDelete.open()

    CategoryListModel {
        id: categoriesModel
    }

    ConfirmDialog {
        id: confirmDelete
        heading: qsTr("Delete this expense?")
        message: qsTr("This cannot be undone.")
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: sheet.removeExpense()
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
