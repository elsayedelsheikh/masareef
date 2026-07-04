import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

// Month-to-date overview: total, budget gauge, top categories and the most
// recent expenses.
Flickable {
    id: screen

    property DashboardViewModel dashboard
    property ExpenseListModel expenses

    signal editRequested(int expenseId)
    signal reportsRequested()

    contentWidth: width
    contentHeight: column.implicitHeight + 2 * Theme.spacingM
    clip: true

    ColumnLayout {
        id: column
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.spacingM
        }
        spacing: Theme.spacingM

        Text {
            text: new Date().toLocaleDateString(Qt.locale(AppBackend.localeName),
                                                "MMMM yyyy")
            font.pixelSize: Theme.fontSizeTitle
            font.weight: Font.DemiBold
            color: Theme.primaryInk
        }

        StatCard {
            Layout.fillWidth: true
            title: qsTr("Spent this month")
            value: screen.dashboard ? screen.dashboard.monthTotalFormatted : ""
            subtitle: screen.dashboard && screen.dashboard.hasBudget
                ? (screen.dashboard.overBudget
                       ? qsTr("Over budget")
                       : qsTr("%1 left of %2")
                             .arg(AppBackend.formatMoney(
                                      screen.dashboard.budgetMinor
                                      - screen.dashboard.spentMinor))
                             .arg(AppBackend.formatMoney(screen.dashboard.budgetMinor)))
                : ""
        }

        BudgetGauge {
            Layout.fillWidth: true
            visible: screen.dashboard && screen.dashboard.hasBudget
            progress: screen.dashboard ? screen.dashboard.progress : 0
            overBudget: screen.dashboard ? screen.dashboard.overBudget : false
            spentText: screen.dashboard
                ? AppBackend.formatMoney(screen.dashboard.spentMinor) : ""
            budgetText: screen.dashboard
                ? AppBackend.formatMoney(screen.dashboard.budgetMinor) : ""
        }

        Button {
            Layout.fillWidth: true
            flat: true
            text: qsTr("View Reports")
            implicitHeight: Theme.touchTarget
            Material.foreground: Theme.accent
            onClicked: screen.reportsRequested()
        }

        Text {
            visible: topRepeater.count > 0
            text: qsTr("Top categories")
            font.pixelSize: Theme.fontSizeCaption
            font.letterSpacing: 0.5
            color: Theme.mutedInk
        }

        Repeater {
            id: topRepeater
            model: screen.dashboard ? screen.dashboard.topCategories : null

            delegate: RowLayout {
                required property var model
                Layout.fillWidth: true
                spacing: Theme.spacingM

                Rectangle {
                    width: 12; height: 12; radius: 6
                    color: model.color
                }
                Text {
                    Layout.fillWidth: true
                    text: model.name
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.primaryInk
                    elide: Text.ElideRight
                }
                Rectangle {
                    Layout.preferredWidth: 90
                    height: 6
                    radius: 3
                    color: Theme.gridline

                    Rectangle {
                        width: parent.width * model.share
                        height: parent.height
                        radius: 3
                        color: model.color
                    }
                }
                Text {
                    text: model.amountFormatted
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.secondaryInk
                }
            }
        }

        Text {
            visible: screen.expenses && screen.expenses.count > 0
            text: qsTr("Recent expenses")
            font.pixelSize: Theme.fontSizeCaption
            font.letterSpacing: 0.5
            color: Theme.mutedInk
        }

        Repeater {
            model: screen.expenses

            // Preview of the newest five rows; the full list lives in the
            // Expenses tab.
            delegate: RowLayout {
                required property var model
                required property int index
                visible: index < 5
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? implicitHeight : 0
                spacing: Theme.spacingM

                Rectangle {
                    width: 10; height: 10; radius: 5
                    color: model.categoryColor
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Text {
                        Layout.fillWidth: true
                        text: model.description.length > 0 ? model.description
                                                           : model.categoryName
                        font.pixelSize: Theme.fontSizeBody
                        color: Theme.primaryInk
                        elide: Text.ElideRight
                    }
                    Text {
                        text: model.date.toLocaleDateString(
                                  Qt.locale(AppBackend.localeName),
                                  Locale.ShortFormat)
                        font.pixelSize: Theme.fontSizeCaption
                        color: Theme.mutedInk
                    }
                }
                Text {
                    text: model.amountFormatted
                    font.pixelSize: Theme.fontSizeBody
                    font.weight: Font.DemiBold
                    color: Theme.primaryInk
                }

                TapHandler {
                    onTapped: screen.editRequested(model.expenseId)
                }
            }
        }

        EmptyState {
            Layout.fillWidth: true
            Layout.topMargin: Theme.spacingXl
            visible: !screen.expenses || screen.expenses.count === 0
            iconSource: "../icons/wallet.svg"
            title: qsTr("No expenses yet")
            hint: qsTr("Tap + to add your first expense")
        }
    }
}
