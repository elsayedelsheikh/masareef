import QtQuick
import QtQuick.Layouts
import Masareef

// Inline single-choice control: a row of equal-width segments.
//
// Preferred over a ComboBox on a phone — every option is visible without a
// popup, it is one tap instead of two, and there is no second popup surface
// to keep mirrored for right-to-left layouts.
Item {
    id: control

    property var options: [] // list of display strings
    property int currentIndex: 0

    implicitHeight: Theme.touchTarget
    implicitWidth: row.implicitWidth + 2 * Theme.spacingM

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusS
        color: "transparent"
        border.color: Theme.gridline
        border.width: 1
    }

    RowLayout {
        id: row

        anchors.fill: parent
        anchors.margins: 3
        spacing: 3

        Repeater {
            model: control.options

            delegate: Rectangle {
                id: segment

                required property int index
                required property string modelData

                readonly property bool selected: control.currentIndex === segment.index

                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: Theme.radiusS - 2
                color: selected ? Theme.primary : "transparent"

                Behavior on color {
                    ColorAnimation {
                        duration: Theme.durationFast
                        easing.type: Theme.easing
                    }
                }

                Text {
                    anchors.centerIn: parent
                    width: parent.width - Theme.spacingS
                    text: segment.modelData
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    font.pixelSize: Theme.fontSizeBody
                    font.weight: segment.selected ? Font.DemiBold : Font.Normal
                    color: segment.selected ? "#ffffff" : Theme.secondaryInk
                }

                TapHandler {
                    onTapped: control.currentIndex = segment.index
                }
            }
        }
    }
}
