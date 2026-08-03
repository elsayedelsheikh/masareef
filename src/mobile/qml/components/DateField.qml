import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Date entry: a field-like button opening a month-grid picker popup.
Button {
    id: field

    property date value: new Date()

    readonly property var appLocale: Qt.locale(AppBackend.localeName)
    readonly property bool isToday: {
        const today = new Date()
        return value.getDate() === today.getDate()
            && value.getMonth() === today.getMonth()
            && value.getFullYear() === today.getFullYear()
    }

    flat: true
    icon.source: "../icons/calendar.svg"
    // localeName is read so the binding re-runs on a language switch;
    // formatDate() is a plain call and would not be a binding dependency.
    // Reading it is all it is for — the date must not depend on its value.
    text: {
        AppBackend.localeName
        return AppBackend.formatDate(value)
    }
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
        Material.roundedScale: Material.MediumScale
        Material.background: Theme.elevatedColor

        contentItem: ColumnLayout {
            spacing: Theme.spacingS

            // A Popup is not an Item; the mirroring attaches to its content.
            LayoutMirroring.enabled: Theme.rtl
            LayoutMirroring.childrenInherit: true

            RowLayout {
                Layout.fillWidth: true
                spacing: 0

                ToolButton {
                    icon.source: "../icons/chevron-left.svg"
                    onClicked: {
                        if (grid.month === 0) { grid.month = 11; grid.year-- }
                        else grid.month--
                    }
                }
                Text {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.touchTarget
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: {
                        AppBackend.localeName
                        return AppBackend.formatMonthYear(
                            new Date(grid.year, grid.month, 1))
                    }
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
                Layout.fillWidth: true
                locale: field.appLocale
                font.pixelSize: Theme.fontSizeCaption
            }

            MonthGrid {
                id: grid

                Layout.fillWidth: true
                locale: field.appLocale

                delegate: Rectangle {
                    id: dayCell

                    required property var model

                    readonly property bool selected:
                        model.day === field.value.getDate()
                        && model.month === field.value.getMonth()
                        && model.year === field.value.getFullYear()
                    readonly property bool inMonth: model.month === grid.month

                    implicitWidth: 40
                    implicitHeight: 40
                    radius: 20
                    color: selected ? Theme.primary : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: dayCell.model.day
                        font.pixelSize: Theme.fontSizeBody
                        font.weight: dayCell.selected ? Font.DemiBold : Font.Normal
                        opacity: dayCell.inMonth ? 1 : 0.35
                        color: dayCell.selected ? "#ffffff" : Theme.primaryInk
                    }

                    TapHandler {
                        onTapped: {
                            field.value = new Date(dayCell.model.year,
                                                   dayCell.model.month,
                                                   dayCell.model.day)
                            popup.close()
                        }
                    }
                }
            }

            Button {
                Layout.fillWidth: true
                flat: true
                text: qsTr("Today")
                implicitHeight: Theme.touchTarget
                onClicked: {
                    field.value = new Date()
                    popup.close()
                }
            }
        }
    }
}
