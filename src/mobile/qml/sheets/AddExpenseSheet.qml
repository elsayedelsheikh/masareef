import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom sheet for adding an expense. Opens clean every time; Save is
// enabled only while the form is valid.
Drawer {
    id: sheet

    property ExpenseController controller

    property alias amountText: form.amountText
    property alias amountInputHints: form.amountInputHints
    property alias descriptionText: form.descriptionText
    property alias notesText: form.notesText
    property alias selectedCategoryId: form.selectedCategoryId
    property alias date: form.date
    readonly property bool canSave: form.valid

    function save() {
        if (!canSave || !controller)
            return
        const id = controller.add(form.selectedCategoryId, form.amountText,
                                  form.descriptionText, form.date, form.notesText)
        if (id > 0)
            close()
    }

    edge: Qt.BottomEdge
    width: parent ? parent.width : 412
    height: Math.min(contentColumn.implicitHeight + 2 * Theme.spacingL,
                     parent ? parent.height * 0.9 : 800)
    Material.roundedScale: Material.LargeScale

    onAboutToShow: {
        form.clear()
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
            text: qsTr("Add expense")
            font.pixelSize: Theme.fontSizeTitle
            font.weight: Font.DemiBold
            color: Theme.primaryInk
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
