import QtQuick
import QtQuick.Controls.Material
import Masareef

// Search box with a leading magnifier and a clear button that appears once
// there is something to clear.
//
// Padding is logical, not visual: Qt Quick Controls do not swap
// left/rightPadding under mirroring, so the swap is explicit here. Anchors
// *do* mirror, which is why the icon and the button below only name one
// side and still land correctly in Arabic.
TextField {
    id: field

    readonly property real iconSpace: 22 + 2 * Theme.spacingS
    readonly property real clearSpace: field.text.length > 0
        ? Theme.touchTarget - Theme.spacingS : Theme.spacingM

    placeholderText: qsTr("Search")
    implicitHeight: Theme.touchTarget
    // Search terms are names, not prose; predictive text mostly gets in the
    // way and the autocorrect popup covers the results.
    inputMethodHints: Qt.ImhNoPredictiveText
    leftPadding: Theme.rtl ? clearSpace : iconSpace
    rightPadding: Theme.rtl ? iconSpace : clearSpace

    ThemedIcon {
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacingS
        anchors.verticalCenter: parent.verticalCenter
        iconName: "search"
        color: Theme.mutedInk
    }

    ToolButton {
        anchors.right: parent.right
        anchors.rightMargin: 0
        anchors.verticalCenter: parent.verticalCenter
        visible: field.text.length > 0
        icon.source: "../icons/close.svg"
        icon.width: 18
        icon.height: 18
        icon.color: Theme.mutedInk
        onClicked: {
            field.clear()
            field.forceActiveFocus()
        }
    }
}
