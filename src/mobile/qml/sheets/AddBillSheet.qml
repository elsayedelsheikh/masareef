import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for adding a recurring bill. Like the expense sheet, the
// draft survives an accidental close — only Cancel and a successful save
// clear it.
BottomSheet {
    id: sheet

    property BillController controller

    property alias amountText: form.amountText
    property alias nameText: form.nameText
    property alias nextDueDate: form.nextDueDate
    property alias selectedCategoryId: form.selectedCategoryId
    property alias selectedRecurrence: form.selectedRecurrence
    property alias notesText: form.notesText
    readonly property bool canSave: form.valid
    property bool resumedDraft: false
    property string errorMessage: ""

    function save() {
        if (!canSave || !controller)
            return
        // selectedRecurrence is the segment index, not a Recurrence value —
        // the form owns the mapping between the two.
        const id = controller.add(form.selectedCategoryId, form.amountText,
                                  form.nameText, form.nextDueDate,
                                  form.recurrenceFor(form.selectedRecurrence),
                                  form.notesText)
        if (id > 0) {
            startOver()
            close()
        } else {
            errorMessage = controller.lastError
        }
    }

    function startOver() {
        form.clear()
        resumedDraft = false
        errorMessage = ""
    }

    heading: qsTr("Add bill")
    acceptEnabled: canSave

    onAccepted: save()
    onCancelled: {
        startOver()
        close()
    }

    onAboutToShow: {
        categoriesModel.refresh()
        errorMessage = ""
        resumedDraft = form.hasContent
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
