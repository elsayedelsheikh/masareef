import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// One expense row: category dot, description, category + amount. Tap to
// edit; swipe fully in either direction to delete (RTL-agnostic).
SwipeDelegate {
    id: delegate

    required property int index
    required property int expenseId
    required property date date
    required property string categoryName
    required property color categoryColor
    required property string description
    required property string amountFormatted

    signal editRequested(int expenseId)
    signal removeRequested(int index)

    width: ListView.view ? ListView.view.width : implicitWidth
    implicitHeight: Math.max(Theme.touchTarget + 12, content.implicitHeight + 2 * Theme.spacingS)

    onClicked: editRequested(expenseId)

    swipe.onCompleted: removeRequested(index)

    contentItem: RowLayout {
        id: content
        spacing: Theme.spacingM

        Rectangle {
            width: 12
            height: 12
            radius: 6
            color: delegate.categoryColor
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
        color: delegate.down ? Theme.gridline : "transparent"
    }
}
