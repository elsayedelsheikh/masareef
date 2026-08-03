import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// The add/edit price book fields, shared by both price item sheets.
ColumnLayout {
    id: form

    property alias nameText: nameField.text
    property alias priceText: priceField.text
    property alias unitText: unitField.text
    property alias selectedCategoryId: categoryPicker.selectedCategoryId
    property var categories // CategoryListModel

    // The category is deliberately not required: a price is worth recording
    // before you have decided which budget bucket it belongs to.
    readonly property bool valid: nameText.trim().length > 0 && priceField.valid

    readonly property bool hasContent: nameText.length > 0 || priceText.length > 0
        || unitText.length > 0 || selectedCategoryId > 0

    function clear() {
        nameText = ""
        priceText = ""
        unitText = ""
        selectedCategoryId = -1
    }

    function focusFirstField() {
        nameField.forceActiveFocus()
    }

    spacing: Theme.spacingM

    TextField {
        id: nameField
        Layout.fillWidth: true
        placeholderText: qsTr("Item name")
        implicitHeight: Theme.fieldHeight
        EnterKey.type: Qt.EnterKeyNext
        onAccepted: priceField.forceActiveFocus()
    }

    AmountField {
        id: priceField
        Layout.fillWidth: true
        placeholderText: qsTr("Price")
        allowZero: true
    }

    FieldLabel { text: qsTr("Unit") }

    TextField {
        id: unitField
        Layout.fillWidth: true
        placeholderText: qsTr("kg, litre, piece…")
        implicitHeight: Theme.fieldHeight
        EnterKey.type: Qt.EnterKeyDone
    }

    // The eight units that cover almost every grocery run. Typing one by
    // hand still works; these just save the keyboard trip, and tapping the
    // selected one again clears it.
    Flow {
        Layout.fillWidth: true
        spacing: Theme.spacingS

        Repeater {
            model: [qsTr("kg"), qsTr("g"), qsTr("litre"), qsTr("ml"),
                    qsTr("piece"), qsTr("pack"), qsTr("box"), qsTr("dozen")]

            delegate: Rectangle {
                id: unitChip

                required property string modelData

                readonly property bool selected: form.unitText === unitChip.modelData

                width: unitChipLabel.implicitWidth + 2 * Theme.spacingM
                height: 34
                radius: height / 2
                color: selected ? Theme.primaryTint : "transparent"
                border.color: selected ? Theme.primary : Theme.gridline
                border.width: selected ? 2 : 1

                Behavior on color {
                    ColorAnimation {
                        duration: Theme.durationFast
                        easing.type: Theme.easing
                    }
                }

                Text {
                    id: unitChipLabel
                    anchors.centerIn: parent
                    text: unitChip.modelData
                    font.pixelSize: Theme.fontSizeBody
                    font.weight: unitChip.selected ? Font.DemiBold : Font.Normal
                    color: unitChip.selected ? Theme.primary : Theme.secondaryInk
                }

                TapHandler {
                    onTapped: form.unitText =
                        unitChip.selected ? "" : unitChip.modelData
                }
            }
        }
    }

    FieldLabel { text: qsTr("Category") }

    CategoryPicker {
        id: categoryPicker
        Layout.fillWidth: true
        model: form.categories
        allowNone: true
    }
}
