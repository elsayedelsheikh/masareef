import QtQuick
import Masareef

// Budget progress bar: green while comfortable, amber near the limit,
// red when over.
Column {
    id: gauge

    property real progress: 0 // 0..1 (already clamped by the viewmodel)
    property bool overBudget: false
    property string spentText
    property string budgetText

    readonly property color barColor: overBudget ? Theme.critical
        : progress >= 0.8 ? Theme.serious
        : Theme.good

    spacing: Theme.spacingS

    Row {
        width: parent.width
        Text {
            width: parent.width / 2
            text: gauge.spentText
            font.pixelSize: Theme.fontSizeBody
            font.weight: Font.DemiBold
            color: gauge.barColor
        }
        Text {
            width: parent.width / 2
            text: gauge.budgetText
            font.pixelSize: Theme.fontSizeBody
            color: Theme.mutedInk
            horizontalAlignment: Text.AlignRight
        }
    }

    Rectangle {
        id: track
        width: parent.width
        height: 10
        radius: height / 2
        color: Theme.gridline

        Rectangle {
            width: Math.max(height, track.width * Math.min(gauge.progress, 1))
            height: parent.height
            radius: height / 2
            color: gauge.barColor

            Behavior on width {
                NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
            }
        }
    }
}
