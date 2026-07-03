import QtQuick
import Masareef

// Rounded card with a caption, a large value and an optional subtitle.
Rectangle {
    id: card

    property alias title: titleLabel.text
    property alias value: valueLabel.text
    property alias subtitle: subtitleLabel.text

    radius: Theme.radiusM
    color: Theme.cardColor
    border.color: Theme.gridline
    border.width: 1
    implicitHeight: column.implicitHeight + 2 * Theme.spacingM

    Column {
        id: column
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            margins: Theme.spacingM
        }
        spacing: Theme.spacingXs

        Text {
            id: titleLabel
            width: parent.width
            font.pixelSize: Theme.fontSizeCaption
            font.letterSpacing: 0.5
            color: Theme.mutedInk
        }
        Text {
            id: valueLabel
            width: parent.width
            font.pixelSize: Theme.fontSizeDisplay
            font.weight: Font.DemiBold
            color: Theme.primaryInk
            elide: Text.ElideRight
        }
        Text {
            id: subtitleLabel
            width: parent.width
            visible: text.length > 0
            font.pixelSize: Theme.fontSizeBody
            color: Theme.secondaryInk
        }
    }
}
