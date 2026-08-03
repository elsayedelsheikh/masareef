import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for adding a price book entry. Like the expense and bill
// sheets, the draft survives an accidental close — only Cancel and a
// successful save clear it.
BottomSheet {
    id: sheet

    property PriceItemController controller

    property alias nameText: form.nameText
    property alias priceText: form.priceText
    property alias unitText: form.unitText
    property alias selectedCategoryId: form.selectedCategoryId
    readonly property bool canSave: form.valid
    property bool resumedDraft: false
    // True while the sheet is showing a copy of an existing item
    property bool prefilled: false
    property string errorMessage: ""

    // Opens the sheet as a copy of an existing row. The name arrives already
    // suffixed by the caller — the price book keys on name, so a straight
    // copy would collide.
    function openCopyOf(name, categoryId, unit, priceText) {
        startOver()
        form.nameText = name
        form.selectedCategoryId = categoryId
        form.unitText = unit
        form.priceText = priceText
        prefilled = true
        open()
    }

    function save() {
        if (!canSave || !controller)
            return
        const id = controller.add(form.nameText, form.selectedCategoryId,
                                  form.unitText, form.priceText)
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

    heading: qsTr("Add item")
    acceptEnabled: canSave

    onAccepted: save()
    onCancelled: {
        startOver()
        close()
    }

    onAboutToShow: {
        categoriesModel.refresh()
        errorMessage = ""
        // A copy was not "left off" anywhere; it says so itself.
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
        message: qsTr("Copied from an existing item. Give it its own name.")
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

    PriceItemForm {
        id: form
        Layout.fillWidth: true
        categories: categoriesModel
    }
}
