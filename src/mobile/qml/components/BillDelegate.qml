import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// One recurring bill: category dot, name, what it is and when it is due,
// then the amount and a Paid button.
//
// The due line carries the urgency color, so an overdue bill reads as
// overdue at a glance rather than only through which section it sits in.
// Tap to edit, long-press for the row's actions.
ItemDelegate {
    id: delegate

    required property int index
    required property int billId
    required property string name
    required property string categoryName
    required property color categoryColor
    required property string amountFormatted
    required property string dueLabel
    required property string recurrenceLabel
    required property string urgency
    required property bool active

    signal editRequested(int index)
    signal menuRequested(int index)
    signal payRequested(int index)

    readonly property color urgencyColor: {
        if (!active)
            return Theme.mutedInk
        switch (urgency) {
        case "overdue":
            return Theme.critical
        case "due":
            return Theme.serious
        default:
            return Theme.mutedInk
        }
    }

    width: ListView.view ? ListView.view.width : implicitWidth
    implicitHeight: Math.max(Theme.touchTarget + 14,
                             content.implicitHeight + 2 * Theme.spacingS)
    padding: Theme.spacingS
    // A paused bill is still listed, just visibly out of the running.
    opacity: active ? 1 : 0.55

    // A long press still ends in a release, which ItemDelegate reports as a
    // click — so without this the actions sheet and the edit sheet would
    // both open. Cleared on every new press, not only on the click that
    // follows, in case the finger is lifted outside the row.
    property bool _longPressed: false

    onPressedChanged: if (pressed) _longPressed = false

    onClicked: {
        if (delegate._longPressed)
            return
        delegate.editRequested(delegate.index)
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onLongPressed: {
            delegate._longPressed = true
            delegate.menuRequested(delegate.index)
        }
    }

    contentItem: RowLayout {
        id: content
        spacing: Theme.spacingM

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 12
            implicitHeight: 12
            radius: 6
            color: delegate.categoryColor
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingS

                Text {
                    Layout.fillWidth: true
                    text: delegate.name
                    font.pixelSize: Theme.fontSizeSubtitle
                    color: Theme.primaryInk
                    elide: Text.ElideRight
                }

                // Only shown when paused: an active bill is the normal case
                // and does not need a badge saying so.
                Rectangle {
                    visible: !delegate.active
                    implicitWidth: pausedLabel.implicitWidth + Theme.spacingM
                    implicitHeight: 20
                    radius: 10
                    color: Theme.gridline

                    Text {
                        id: pausedLabel
                        anchors.centerIn: parent
                        text: qsTr("Paused")
                        font.pixelSize: Theme.fontSizeCaption
                        color: Theme.secondaryInk
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: delegate.categoryName + " · " + delegate.recurrenceLabel
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.mutedInk
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: delegate.dueLabel
                font.pixelSize: Theme.fontSizeCaption
                font.weight: delegate.urgency === "overdue" ? Font.DemiBold
                                                            : Font.Normal
                color: delegate.urgencyColor
                elide: Text.ElideRight
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 0

            Text {
                Layout.alignment: Qt.AlignRight
                text: delegate.amountFormatted
                font.pixelSize: Theme.fontSizeSubtitle
                font.weight: Font.DemiBold
                color: Theme.primaryInk
            }

            // Logs an expense for this bill and advances its next due date.
            // Paused bills have no next date to advance to, so the button
            // goes away rather than failing.
            Button {
                Layout.alignment: Qt.AlignRight
                visible: delegate.active
                flat: true
                text: qsTr("Paid")
                font.pixelSize: Theme.fontSizeBody
                implicitHeight: Theme.touchTarget - Theme.spacingS
                Material.foreground: Theme.accent
                onClicked: delegate.payRequested(delegate.index)
            }
        }
    }

    background: Rectangle {
        radius: Theme.radiusS
        color: delegate.down ? Theme.pressedTint : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: Theme.durationFast
                easing.type: Theme.easing
            }
        }
    }
}
