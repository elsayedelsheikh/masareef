import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Themed confirmation dialog. Centralized so every destructive prompt looks
// and reads the same.
//
// The heading lives in the content rather than in Dialog.title: the title is
// part of the popup chrome and does not pick up the LayoutMirroring that has
// to be attached to the content root (a Popup is not an Item), so an Arabic
// UI would leave it stranded on the wrong side.
Dialog {
    id: control

    property string heading
    property string message
    property string confirmText: qsTr("OK")
    property bool destructive: false

    signal confirmed()

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(360, parent ? parent.width - 2 * Theme.spacingM : 360)
    modal: true
    padding: Theme.spacingL
    Material.roundedScale: Material.MediumScale
    Material.background: Theme.elevatedColor

    contentItem: ColumnLayout {
        spacing: Theme.spacingS

        LayoutMirroring.enabled: Theme.rtl
        LayoutMirroring.childrenInherit: true

        Text {
            Layout.fillWidth: true
            text: control.heading
            font.pixelSize: Theme.fontSizeSubtitle
            font.weight: Font.DemiBold
            color: Theme.primaryInk
            wrapMode: Text.WordWrap
        }

        Text {
            Layout.fillWidth: true
            visible: text.length > 0
            text: control.message
            font.pixelSize: Theme.fontSizeBody
            color: Theme.secondaryInk
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingS
            spacing: Theme.spacingS

            Item { Layout.fillWidth: true }

            Button {
                flat: true
                text: qsTr("Cancel")
                implicitHeight: Theme.touchTarget
                onClicked: control.close()
            }

            Button {
                flat: true
                text: control.confirmText
                implicitHeight: Theme.touchTarget
                Material.foreground: control.destructive ? Theme.critical
                                                         : Theme.primary
                onClicked: {
                    control.confirmed()
                    control.close()
                }
            }
        }
    }
}
