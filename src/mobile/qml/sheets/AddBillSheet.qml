import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for adding a recurring bill.
Drawer {
    id: sheet

    property BillController controller

    property alias amountText: amountField.text
    property alias nameText: nameField.text
    property alias nextDueDate: dateField.value
    property alias selectedCategoryId: categoryPicker.selectedCategoryId
    property alias selectedRecurrence: recurrenceCombo.currentIndex
    property alias notesText: notesField.text
    readonly property bool canSave: amountText && nameText && nextDueDate &&
                                     selectedCategoryId > 0

    function save() {
        if (!canSave || !controller)
            return
        const recurrences = [Recurrence.Monthly, Recurrence.Quarterly, Recurrence.Yearly]
        const id = controller.add(selectedCategoryId, amountText, nameText,
                                  nextDueDate, recurrences[selectedRecurrence],
                                  notesText)
        if (id > 0)
            close()
    }

    edge: Qt.BottomEdge
    width: parent ? parent.width : 412
    height: Math.min(contentColumn.implicitHeight + 2 * Theme.spacingL,
                     parent ? parent.height * 0.9 : 800)
    Material.roundedScale: Material.LargeScale

    onAboutToShow: {
        amountText = ""
        nameText = ""
        selectedCategoryId = -1
        selectedRecurrence = 0 // Monthly
        notesText = ""
        categoriesModel.refresh()
    }

    CategoryListModel {
        id: categoriesModel
    }

    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        anchors.margins: Theme.spacingL
        spacing: Theme.spacingM

        Text {
            text: qsTr("Add bill")
            font.pixelSize: Theme.fontSizeTitle
            font.weight: Font.DemiBold
            color: Theme.primaryInk
        }

        CategoryPicker {
            id: categoryPicker
            Layout.fillWidth: true
            model: categoriesModel
        }

        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: qsTr("Bill name")
            implicitHeight: Theme.touchTarget
        }

        AmountField {
            id: amountField
            Layout.fillWidth: true
        }

        DateField {
            id: dateField
            Layout.fillWidth: true
            label: qsTr("Next due")
        }

        ComboBox {
            id: recurrenceCombo
            Layout.fillWidth: true
            model: [qsTr("Monthly"), qsTr("Quarterly"), qsTr("Yearly")]
            currentIndex: 0
        }

        TextField {
            id: notesField
            Layout.fillWidth: true
            placeholderText: qsTr("Notes (optional)")
            implicitHeight: Theme.touchTarget
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
