import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Bottom action list — the mobile answer to a right-click menu, opened by a
// long press.
//
// A Menu would be one line of code, but its popup sits outside the mirrored
// item tree, so in Arabic every entry would stay left-aligned with its icon
// on the wrong side. This is a Drawer whose content root carries the
// mirroring, like the other sheets.
Drawer {
    id: sheet

    property string heading
    property string subheading
    // [{ actionId, text, icon, destructive }] — `icon` is a bare icon name
    // ("pencil"); ThemedIcon resolves it against qml/icons, so callers do
    // not have to get a relative path right from their own directory.
    property var actions: []

    signal triggered(string actionId)

    function openWith(title, subtitle, entries) {
        heading = title
        subheading = subtitle === undefined ? "" : subtitle
        actions = entries
        open()
    }

    edge: Qt.BottomEdge
    width: parent ? parent.width : 412
    height: Math.min(layout.implicitHeight + 2 * Theme.spacingL,
                     parent ? parent.height * 0.8 : 600)
    padding: 0
    Material.roundedScale: Material.LargeScale
    Material.background: Theme.elevatedColor

    ColumnLayout {
        id: layout

        anchors.fill: parent
        anchors.margins: Theme.spacingL
        spacing: Theme.spacingS

        LayoutMirroring.enabled: Theme.rtl
        LayoutMirroring.childrenInherit: true

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Theme.spacingS
            implicitWidth: 36
            implicitHeight: 4
            radius: 2
            color: Theme.gridline
        }

        Text {
            Layout.fillWidth: true
            visible: text.length > 0
            text: sheet.heading
            font.pixelSize: Theme.fontSizeSubtitle
            font.weight: Font.DemiBold
            color: Theme.primaryInk
            elide: Text.ElideRight
        }

        Text {
            Layout.fillWidth: true
            Layout.bottomMargin: Theme.spacingS
            visible: text.length > 0
            text: sheet.subheading
            font.pixelSize: Theme.fontSizeBody
            color: Theme.mutedInk
            elide: Text.ElideRight
        }

        Repeater {
            model: sheet.actions

            delegate: ItemDelegate {
                id: entry

                required property var modelData

                Layout.fillWidth: true
                implicitHeight: Theme.touchTarget + Theme.spacingS

                onClicked: {
                    sheet.close()
                    sheet.triggered(entry.modelData.actionId)
                }

                contentItem: RowLayout {
                    spacing: Theme.spacingM

                    ThemedIcon {
                        visible: iconName.length > 0
                        iconName: entry.modelData.icon !== undefined
                            ? entry.modelData.icon : ""
                        color: entry.modelData.destructive === true
                            ? Theme.critical : Theme.secondaryInk
                    }

                    Text {
                        Layout.fillWidth: true
                        text: entry.modelData.text
                        font.pixelSize: Theme.fontSizeSubtitle
                        color: entry.modelData.destructive === true
                            ? Theme.critical : Theme.primaryInk
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}
