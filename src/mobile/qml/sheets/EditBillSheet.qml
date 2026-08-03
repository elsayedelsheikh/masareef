import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for editing a recurring bill. Call openFor(id).
BottomSheet {
    id: sheet

    property BillController controller
    property int billId: -1

    property alias amountText: form.amountText
    property alias nameText: form.nameText
    property alias nextDueDate: form.nextDueDate
    property alias selectedCategoryId: form.selectedCategoryId
    property alias selectedRecurrence: form.selectedRecurrence
    property alias notesText: form.notesText
    readonly property bool canSave: form.valid
    property string errorMessage: ""

    function openFor(id) {
        if (!controller)
            return
        errorMessage = ""
        categoriesModel.refresh()
        // Load before committing to the id: if the bill is gone (deleted on
        // another screen) the sheet must not open bound to nothing.
        if (!controller.load(id))
            return
        billId = id
        form.amountText = controller.editAmountText
        form.nameText = controller.editName
        form.selectedCategoryId = controller.editCategoryId
        form.selectedRecurrence = controller.editRecurrence
        form.notesText = controller.editNotes
        form.nextDueDate = controller.editNextDue
        open()
    }

    function save() {
        if (!canSave || !controller || billId <= 0)
            return
        if (controller.update(billId, form.selectedCategoryId, form.amountText,
                              form.nameText, form.nextDueDate,
                              form.selectedRecurrence, form.notesText))
            close()
        else
            errorMessage = controller.lastError
    }

    function removeBill() {
        if (!controller || billId <= 0)
            return
        if (controller.remove(billId))
            close()
        else
            errorMessage = controller.lastError
    }

    heading: qsTr("Edit bill")
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
        heading: qsTr("Delete this bill?")
        message: qsTr("Expenses already logged for it are kept.")
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: sheet.removeBill()
    }

    InlineBanner {
        Layout.fillWidth: true
        visible: sheet.errorMessage.length > 0
        message: sheet.errorMessage
        tint: Theme.criticalTint
        ink: Theme.critical
    }

    BillForm {
        id: form
        Layout.fillWidth: true
        categories: categoriesModel
    }
}
