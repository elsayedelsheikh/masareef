import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// One price book row: category dot, item name, unit and last-updated date,
// then the price with a badge for which way it last moved.
//
// Tap to edit, long-press for the row's actions. Unlike an expense row this
// one deliberately does not swipe to delete: an expense delete is undoable
// from the snackbar, whereas losing a price takes its whole history with
// it, so deleting stays behind a long press and a confirmation.
ItemDelegate {
    id: delegate

    required property int index
    required property int priceItemId
    required property string name
    required property string unit
    required property string priceFormatted
    required property string categoryName
    required property color categoryColor
    required property bool hasCategory
    required property string updatedAtFormatted
    required property bool hasPreviousPrice
    required property string previousPriceFormatted
    required property bool priceRose
    required property bool priceFell
    required property real changePercent

    // Both carry the row index, not the id: the screen has to look the row
    // up in the model anyway to fill a sheet from it.
    signal editRequested(int index)
    signal menuRequested(int index)

    // A rise is bad news for a shopping list, so it takes the warning color
    // and a fall takes the good one — the opposite of a stock ticker.
    readonly property color trendColor: priceRose ? Theme.critical
                                      : priceFell ? Theme.good
                                      : Theme.mutedInk
    readonly property bool showsTrend:
        hasPreviousPrice && Math.abs(changePercent) >= 0.5

    // "per kg · 12/03/2026", dropping either half when it is missing
    readonly property string subtitle: {
        const parts = []
        if (unit.length > 0)
            parts.push(qsTr("per %1").arg(unit))
        if (hasCategory && categoryName.length > 0)
            parts.push(categoryName)
        if (updatedAtFormatted.length > 0)
            parts.push(updatedAtFormatted)
        return parts.join(" · ")
    }

    width: ListView.view ? ListView.view.width : implicitWidth
    implicitHeight: Math.max(Theme.touchTarget + 14,
                             content.implicitHeight + 2 * Theme.spacingS)
    padding: Theme.spacingS

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

            Text {
                Layout.fillWidth: true
                text: delegate.name
                font.pixelSize: Theme.fontSizeSubtitle
                color: Theme.primaryInk
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                visible: text.length > 0
                text: delegate.subtitle
                font.pixelSize: Theme.fontSizeCaption
                color: Theme.mutedInk
                elide: Text.ElideRight
            }
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            spacing: 2

            Text {
                Layout.alignment: Qt.AlignRight
                text: delegate.priceFormatted
                font.pixelSize: Theme.fontSizeSubtitle
                font.weight: Font.DemiBold
                color: Theme.primaryInk
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                visible: delegate.showsTrend
                spacing: Theme.spacingXs

                ThemedIcon {
                    iconName: delegate.priceRose ? "trending-up" : "trending-down"
                    size: 14
                    color: delegate.trendColor
                }

                Text {
                    // One decimal would be noise on a shopping list; the
                    // point is "this got dearer", not by how much exactly.
                    text: Math.round(Math.abs(delegate.changePercent)) + "%"
                    font.pixelSize: Theme.fontSizeCaption
                    font.weight: Font.DemiBold
                    color: delegate.trendColor
                }
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
