import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for editing an existing expense. Call openFor(id).
Drawer {
    id: sheet

    property ExpenseController controller
    property int expenseId: 0

    readonly property bool canSave: form.valid

    function openFor(id) {
        expenseId = id
        categoriesModel.refresh()
        if (controller && controller.load(id)) {
            form.amountText = controller.editAmountText
            form.selectedCategoryId = controller.editCategoryId
            form.descriptionText = controller.editDescription
            form.notesText = controller.editNotes
            form.date = controller.editDate
            open()
        }
    }

    function save() {
        if (!canSave || !controller)
            return
        if (controller.update(expenseId, form.selectedCategoryId, form.amountText,
                              form.descriptionText, form.date, form.notesText))
            close()
    }

    function removeExpense() {
        if (controller && controller.remove(expenseId))
            close()
    }

    edge: Qt.BottomEdge
    width: parent ? parent.width : 412
    height: Math.min(contentColumn.implicitHeight + 2 * Theme.spacingL,
                     parent ? parent.height * 0.9 : 800)
    Material.roundedScale: Material.LargeScale

    CategoryListModel {
        id: categoriesModel
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: Theme.spacingL
        spacing: Theme.spacingM

        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                text: qsTr("Edit expense")
                font.pixelSize: Theme.fontSizeTitle
                font.weight: Font.DemiBold
                color: Theme.primaryInk
            }
            ToolButton {
                icon.source: "../icons/trash.svg"
                icon.color: Theme.critical
                onClicked: sheet.removeExpense()
            }
        }

        ExpenseForm {
            id: form
            Layout.fillWidth: true
            categories: categoriesModel
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingM

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Cancel")
                implicitHeight: Theme.touchTarget
                onClicked: sheet.close()
            }
            Button {
                Layout.fillWidth: true
                highlighted: true
                enabled: sheet.canSave
                text: qsTr("Save")
                implicitHeight: Theme.touchTarget
                onClicked: sheet.save()
            }
        }
    }
}
