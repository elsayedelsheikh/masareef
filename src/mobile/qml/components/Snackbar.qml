import QtQuick
import QtQuick.Controls.Material
import Masareef

// Transient bottom notice with an optional action (e.g. Undo).
Popup {
    id: snackbar

    property alias text: messageLabel.text
    property alias actionText: actionButton.text

    signal actionTriggered()

    function show(message, action) {
        text = message
        actionText = action === undefined ? "" : action
        open()
        hideTimer.restart()
    }

    parent: Overlay.overlay
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? parent.height - height - Theme.spacingXl - Theme.touchTarget : 0
    padding: Theme.spacingS
    closePolicy: Popup.NoAutoClose
    Material.background: Theme.dark ? "#3a3a38" : "#323230"

    Timer {
        id: hideTimer
        interval: 4000
        onTriggered: snackbar.close()
    }

    contentItem: Row {
        spacing: Theme.spacingM

        Text {
            id: messageLabel
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: Theme.fontSizeBody
            color: "#ffffff"
        }
        Button {
            id: actionButton
            visible: text.length > 0
            anchors.verticalCenter: parent.verticalCenter
            flat: true
            Material.foreground: Theme.accent
            onClicked: {
                hideTimer.stop()
                snackbar.close()
                snackbar.actionTriggered()
            }
        }
    }
}
