import QtQuick
import Masareef

// Centered illustration + message for empty lists.
//
// Anchor this beside a list, not inside it: a ListView's children become
// children of its content item, which is zero-high while the list is empty
// — exactly when the empty state has to be visible.
Column {
    id: empty

    // Bare icon name ("receipt"); see ThemedIcon.
    property alias iconName: icon.iconName
    property alias title: titleLabel.text
    property alias hint: hintLabel.text

    // Explicit, so the labels can bind their width to the column without the
    // column sizing itself from them. Callers usually override it.
    width: 260
    spacing: Theme.spacingS

    ThemedIcon {
        id: icon
        anchors.horizontalCenter: parent.horizontalCenter
        size: 72
        color: Theme.mutedInk
        opacity: 0.6
    }
    // Both labels span the column and center their own text, so a long hint
    // wraps instead of pushing the layout wider than the screen.
    Text {
        id: titleLabel
        width: empty.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeSubtitle
        font.weight: Font.DemiBold
        color: Theme.secondaryInk
    }
    Text {
        id: hintLabel
        width: empty.width
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        font.pixelSize: Theme.fontSizeBody
        color: Theme.mutedInk
    }
}
