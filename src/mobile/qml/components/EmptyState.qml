import QtQuick
import QtQuick.Controls.impl as Impl
import Masareef

// Centered illustration + message for empty lists.
Column {
    id: empty

    property alias iconSource: icon.source
    property alias title: titleLabel.text
    property alias hint: hintLabel.text

    spacing: Theme.spacingS

    Impl.IconImage {
        id: icon
        anchors.horizontalCenter: parent.horizontalCenter
        sourceSize: Qt.size(72, 72)
        color: Theme.mutedInk
        opacity: 0.6
    }
    Text {
        id: titleLabel
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: Theme.fontSizeSubtitle
        font.weight: Font.DemiBold
        color: Theme.secondaryInk
    }
    Text {
        id: hintLabel
        anchors.horizontalCenter: parent.horizontalCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: Theme.fontSizeBody
        color: Theme.mutedInk
    }
}
