import QtQuick
import QtQuick.Controls.Material
import Masareef

// Date entry: a field-like button opening a month-grid picker popup.
Button {
    id: field

    property date value: new Date()

    readonly property var appLocale: Qt.locale(AppBackend.localeName)

    flat: true
    icon.source: "../icons/calendar.svg"
    text: value.toLocaleDateString(appLocale, Locale.LongFormat)
    font.pixelSize: Theme.fontSizeBody
    implicitHeight: Theme.touchTarget

    onClicked: {
        grid.month = value.getMonth()
        grid.year = value.getFullYear()
        popup.open()
    }

    Popup {
        id: popup

        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(360, parent ? parent.width - 2 * Theme.spacingM : 360)
        modal: true
        padding: Theme.spacingM

        contentItem: Column {
            spacing: Theme.spacingS

            Row {
                width: parent.width

                ToolButton {
                    icon.source: "../icons/chevron-left.svg"
                    onClicked: {
                        if (grid.month === 0) { grid.month = 11; grid.year-- }
                        else grid.month--
                    }
                }
                Text {
                    width: parent.width - 2 * Theme.touchTarget
                    height: Theme.touchTarget
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: new Date(grid.year, grid.month, 1)
                        .toLocaleDateString(field.appLocale, "MMMM yyyy")
                    font.pixelSize: Theme.fontSizeSubtitle
                    font.weight: Font.DemiBold
                    color: Theme.primaryInk
                }
                ToolButton {
                    icon.source: "../icons/chevron-right.svg"
                    onClicked: {
                        if (grid.month === 11) { grid.month = 0; grid.year++ }
                        else grid.month++
                    }
                }
            }

            DayOfWeekRow {
                width: parent.width
                locale: field.appLocale
                font.pixelSize: Theme.fontSizeCaption
            }

            MonthGrid {
                id: grid
                width: parent.width
                locale: field.appLocale

                delegate: Rectangle {
                    required property var model

                    readonly property bool selected:
                        model.day === field.value.getDate()
                        && model.month === field.value.getMonth()
                        && model.year === field.value.getFullYear()

                    implicitWidth: 40
                    implicitHeight: 40
                    radius: 20
                    color: selected ? Theme.primary : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: model.day
                        font.pixelSize: Theme.fontSizeBody
                        opacity: model.month === grid.month ? 1 : 0.35
                        color: parent.selected ? "#ffffff" : Theme.primaryInk
                    }

                    TapHandler {
                        onTapped: {
                            field.value = new Date(model.year, model.month, model.day)
                            popup.close()
                        }
                    }
                }
            }
        }
    }
}
