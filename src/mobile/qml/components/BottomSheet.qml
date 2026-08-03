import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Shared chrome for the add/edit sheets: a grab handle, a title row with an
// optional destructive action, a scrollable body and a Cancel/Save footer.
//
// These sheets never dismiss themselves. A Drawer drags to close and closes
// on a tap outside by default, which on a phone means a stray gesture while
// reaching for a field throws away a half-typed entry — so both are off
// here, and only Cancel, Save, Escape or the Android back button close one.
// The sheets that own a form also keep the draft, so back-then-reopen
// resumes where it left off instead of starting over.
Drawer {
    id: sheet

    property string heading
    property string acceptText: qsTr("Save")
    property bool acceptEnabled: true
    // Shows a destructive action in the title row (the edit sheets)
    property bool deletable: false
    default property alias body: bodyColumn.data

    signal accepted()
    signal cancelled()
    signal deleteRequested()

    // How much of the sheet the software keyboard covers. Clamped to half
    // the window: a bogus keyboardRectangle must not be able to collapse the
    // layout, and the value is 0 anyway when Android resizes the window
    // instead (then the drawer is already sitting above the keyboard).
    readonly property real keyboardInset: {
        if (!Qt.inputMethod.visible || !sheet.parent)
            return 0
        const keyboard = Qt.inputMethod.keyboardRectangle
        if (keyboard.height <= 0)
            return 0
        return Math.max(0, Math.min(sheet.parent.height * 0.5,
                                    sheet.parent.height - keyboard.y))
    }

    edge: Qt.BottomEdge
    width: parent ? parent.width : 412
    height: Math.min(layout.implicitHeight + 2 * Theme.spacingL + keyboardInset,
                     parent ? parent.height * 0.94 : 800)
    interactive: false                  // no drag-to-dismiss
    closePolicy: Popup.CloseOnEscape    // no close-on-tap-outside
    padding: 0
    Material.roundedScale: Material.LargeScale
    Material.background: Theme.elevatedColor

    Behavior on height {
        NumberAnimation {
            duration: Theme.durationFast
            easing.type: Theme.easing
        }
    }

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: Theme.spacingL
        // Lift the footer clear of the keyboard when Android pans instead of
        // resizing; `keyboardInset` is 0 in the resize case.
        anchors.bottomMargin: Theme.spacingL + sheet.keyboardInset
        spacing: Theme.spacingM

        // A Popup is not an Item, so the mirroring has to be attached here,
        // on the content root, rather than to the sheet itself.
        LayoutMirroring.enabled: Theme.rtl
        LayoutMirroring.childrenInherit: true

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Theme.spacingXs
            implicitWidth: 36
            implicitHeight: 4
            radius: 2
            color: Theme.gridline
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingS

            Text {
                Layout.fillWidth: true
                text: sheet.heading
                font.pixelSize: Theme.fontSizeTitle
                font.weight: Font.DemiBold
                color: Theme.primaryInk
                elide: Text.ElideRight
            }

            ToolButton {
                visible: sheet.deletable
                icon.source: "../icons/trash.svg"
                icon.color: Theme.critical
                onClicked: sheet.deleteRequested()
            }
        }

        Flickable {
            id: bodyFlick

            Layout.fillWidth: true
            Layout.fillHeight: true
            // Sizes the sheet to its content; fillHeight lets it shrink and
            // scroll once the content no longer fits (small screen, or the
            // keyboard eating half of it).
            Layout.preferredHeight: bodyColumn.implicitHeight
            contentWidth: width
            contentHeight: bodyColumn.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick

            ScrollBar.vertical: ScrollBar {
                policy: bodyFlick.contentHeight > bodyFlick.height
                    ? ScrollBar.AsNeeded : ScrollBar.AlwaysOff
            }

            ColumnLayout {
                id: bodyColumn
                width: bodyFlick.width
                spacing: Theme.spacingM
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingXs
            spacing: Theme.spacingM

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Cancel")
                implicitHeight: Theme.touchTarget
                onClicked: sheet.cancelled()
            }

            Button {
                Layout.fillWidth: true
                highlighted: true
                enabled: sheet.acceptEnabled
                text: sheet.acceptText
                implicitHeight: Theme.touchTarget
                onClicked: sheet.accepted()
            }
        }
    }
}
