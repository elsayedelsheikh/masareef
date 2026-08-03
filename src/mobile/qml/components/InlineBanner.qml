import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Tinted inline notice with an optional action.
//
// Used instead of a Snackbar wherever the message belongs to a modal
// surface: a snackbar lives in the window overlay and would be covered by
// the sheet that raised it.
Rectangle {
    id: banner

    property string message
    property string actionText
    property color tint: Theme.primaryTint
    property color ink: Theme.primary

    signal actionTriggered()

    implicitHeight: row.implicitHeight + 2 * Theme.spacingS
    radius: Theme.radiusS
    color: tint

    RowLayout {
        id: row

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Theme.spacingM
        anchors.rightMargin: Theme.spacingS
        spacing: Theme.spacingS

        Text {
            Layout.fillWidth: true
            text: banner.message
            // Messages carry item names and database error text; a stray
            // "<" in one must read as a "<", not open a tag.
            textFormat: Text.PlainText
            font.pixelSize: Theme.fontSizeBody
            color: banner.ink
            wrapMode: Text.WordWrap
        }

        Button {
            visible: banner.actionText.length > 0
            flat: true
            text: banner.actionText
            font.pixelSize: Theme.fontSizeBody
            implicitHeight: Theme.touchTarget - Theme.spacingS
            Material.foreground: banner.ink
            onClicked: banner.actionTriggered()
        }
    }
}
