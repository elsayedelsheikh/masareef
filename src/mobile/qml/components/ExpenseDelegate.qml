import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// One expense row: category dot, description, category + amount.
//
// Tap to edit, long-press for the action sheet (edit / duplicate / select /
// delete), swipe fully in either direction to delete (RTL-agnostic).
SwipeDelegate {
    id: delegate

    required property int index
    required property int expenseId
    required property date date
    required property string categoryName
    required property color categoryColor
    required property string description
    required property string amountFormatted

    property bool isSelected: false
    property bool selectionMode: false

    signal editRequested(int expenseId)
    signal removeRequested(int index)
    signal selectionToggled(int index)
    signal menuRequested(int index)

    width: ListView.view ? ListView.view.width : implicitWidth
    implicitHeight: Math.max(Theme.touchTarget + 12,
                             content.implicitHeight + 2 * Theme.spacingS)
    padding: Theme.spacingS

    // A long press still ends in a release, which SwipeDelegate reports as
    // a click — so without this the long press would be undone by the tap
    // that ends it (in selection mode) or open a sheet on top of the
    // actions. Cleared on every new press, in case the finger is lifted
    // outside the row.
    property bool _longPressed: false

    onPressedChanged: if (pressed) _longPressed = false

    onClicked: {
        if (delegate._longPressed)
            return
        if (selectionMode)
            selectionToggled(index)
        else
            editRequested(expenseId)
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        // In selection mode a long press keeps extending the selection;
        // otherwise it opens the row's actions.
        onLongPressed: {
            delegate._longPressed = true
            if (delegate.selectionMode)
                delegate.selectionToggled(delegate.index)
            else
                delegate.menuRequested(delegate.index)
        }
    }

    swipe.onCompleted: {
        if (!selectionMode)
            removeRequested(index)
    }

    contentItem: RowLayout {
        id: content
        spacing: Theme.spacingM

        Rectangle {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 12
            implicitHeight: 12
            radius: 6
            color: delegate.isSelected ? Theme.primary : delegate.categoryColor

            Behavior on color {
                ColorAnimation {
                    duration: Theme.durationFast
                    easing.type: Theme.easing
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: delegate.description.length > 0 ? delegate.description
                                                      : delegate.categoryName
                font.pixelSize: Theme.fontSizeSubtitle
                color: Theme.primaryInk
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: delegate.categoryName
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.mutedInk
                elide: Text.ElideRight
            }
        }

        Text {
            text: delegate.amountFormatted
            font.pixelSize: Theme.fontSizeSubtitle
            font.weight: Font.DemiBold
            color: Theme.primaryInk
        }
    }

    swipe.behind: Rectangle {
        width: delegate.width
        height: delegate.height
        color: Theme.critical

        Image {
            anchors.centerIn: parent
            source: "../icons/trash.svg"
            sourceSize: Qt.size(24, 24)
        }
    }

    background: Rectangle {
        radius: Theme.radiusS
        color: delegate.isSelected ? Theme.primaryTint
             : delegate.down ? Theme.pressedTint
             : "transparent"

        Behavior on color {
            ColorAnimation {
                duration: Theme.durationFast
                easing.type: Theme.easing
            }
        }
    }
}
