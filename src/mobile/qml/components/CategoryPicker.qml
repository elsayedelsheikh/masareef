import QtQuick
import QtQuick.Controls.Material
import Masareef

// Category choice chips. Set allowAll to show a leading "All" chip that
// maps to categoryId -1 (used by the expense list filter).
Flow {
    id: picker

    property var model // CategoryListModel
    property int selectedCategoryId: -1
    property bool allowAll: false

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
        color: selected ? Qt.alpha(Theme.primary, 0.18) : "transparent"
        border.color: selected ? Theme.primary : Theme.gridline
        border.width: 1

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
        }
    }
}
