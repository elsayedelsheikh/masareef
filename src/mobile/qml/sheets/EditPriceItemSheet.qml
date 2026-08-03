import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for editing a price book entry. Call openFor(id).
BottomSheet {
    id: sheet

    property PriceItemController controller
    property int priceItemId: -1
    // Optional context from the row that was tapped, shown while editing so
    // a price change can be judged against what it used to be.
    property string previousPriceText: ""

    property alias nameText: form.nameText
    property alias priceText: form.priceText
    property alias unitText: form.unitText
    property alias selectedCategoryId: form.selectedCategoryId
    readonly property bool canSave: form.valid
    property string errorMessage: ""

    // The load failed, so this sheet never opens and its own banner would
    // never be seen — whoever asked for it has to say so instead.
    signal openFailed(string message)

    function openFor(id, previousPrice) {
        if (!controller)
            return
        errorMessage = ""
        previousPriceText = previousPrice === undefined ? "" : previousPrice
        categoriesModel.refresh()
        if (!controller.load(id)) {
            // The row is gone (deleted on another screen); say so rather
            // than opening a sheet bound to nothing.
            openFailed(controller.lastError)
            return
        }
        priceItemId = id
        form.nameText = controller.editName
        form.priceText = controller.editPriceText
        form.unitText = controller.editUnit
        form.selectedCategoryId = controller.editCategoryId
        open()
    }

    function save() {
        if (!canSave || !controller || priceItemId <= 0)
            return
        if (controller.update(priceItemId, form.nameText,
                              form.selectedCategoryId, form.unitText,
                              form.priceText))
            close()
        else
            errorMessage = controller.lastError
    }

    function removeItem() {
        if (!controller || priceItemId <= 0)
            return
        if (controller.remove(priceItemId))
            close()
        else
            errorMessage = controller.lastError
    }

    heading: qsTr("Edit item")
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
        heading: qsTr("Delete this item?")
        message: qsTr("Its price history is deleted with it.")
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: sheet.removeItem()
    }

    InlineBanner {
        Layout.fillWidth: true
        visible: sheet.errorMessage.length > 0
        message: sheet.errorMessage
        tint: Theme.criticalTint
        ink: Theme.critical
    }

    InlineBanner {
        Layout.fillWidth: true
        visible: sheet.errorMessage.length === 0
                 && sheet.previousPriceText.length > 0
        message: qsTr("Was %1").arg(sheet.previousPriceText)
    }

    PriceItemForm {
        id: form
        Layout.fillWidth: true
        categories: categoriesModel
    }
}
