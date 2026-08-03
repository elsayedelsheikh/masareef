import QtQuick
import QtQuick.Controls.Material
import Masareef

// Category choice chips.
//
// `allowAll` prepends an "All" chip mapping to categoryId -1 (the expense
// list filter); `allowNone` prepends "Uncategorized", also -1, for the price
// book, where an item does not have to belong to a category.
Flow {
    id: picker

    property var model // CategoryListModel
    property int selectedCategoryId: -1
    property bool allowAll: false
    property bool allowNone: false
    property int excludeCategoryId: -1

    spacing: Theme.spacingS

    component Chip: Rectangle {
        id: chip

        property int chipCategoryId: -1
        property alias label: chipLabel.text
        property color dotColor: "transparent"
        property bool showDot: false

        readonly property bool selected: picker.selectedCategoryId === chipCategoryId

        radius: height / 2
        height: 40
        width: chipRow.implicitWidth + 2 * Theme.spacingM
        color: selected ? Theme.primaryTint : "transparent"
        border.color: selected ? Theme.primary : Theme.gridline
        border.width: selected ? 2 : 1

        Behavior on color {
            ColorAnimation {
                duration: Theme.durationFast
                easing.type: Theme.easing
            }
        }

        Row {
            id: chipRow
            anchors.centerIn: parent
            spacing: Theme.spacingS

            Rectangle {
                visible: chip.showDot
                anchors.verticalCenter: parent.verticalCenter
                width: 10
                height: 10
                radius: 5
                color: chip.dotColor
            }
            Text {
                id: chipLabel
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Theme.fontSizeBody
                font.weight: chip.selected ? Font.DemiBold : Font.Normal
                color: chip.selected ? Theme.primary : Theme.primaryInk
            }
        }

        TapHandler {
            onTapped: picker.selectedCategoryId = chip.chipCategoryId
        }
    }

    Chip {
        visible: picker.allowAll
        chipCategoryId: -1
        label: qsTr("All")
    }

    Chip {
        visible: picker.allowNone
        chipCategoryId: -1
        label: qsTr("Uncategorized")
    }

    Repeater {
        model: picker.model

        // Bind through `model.*`: the "color" role would clash with
        // Rectangle.color as a required property.
        delegate: Chip {
            required property var model
            chipCategoryId: model.categoryId
            label: model.name
            showDot: true
            dotColor: model.color
            visible: model.categoryId !== picker.excludeCategoryId
        }
    }
}
