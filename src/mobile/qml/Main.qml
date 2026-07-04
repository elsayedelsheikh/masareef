import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Masareef

ApplicationWindow {
    id: window

    width: 412
    height: 892
    visible: true
    title: qsTr("Masareef")
    color: Theme.surface

    Material.theme: Theme.dark ? Material.Dark : Material.Light
    Material.primary: Theme.primary
    Material.accent: Theme.accent

    // Android back button: close sheets / leave sub-pages before exiting
    onClosing: (close) => {
        if (addSheet.opened) {
            addSheet.close()
            close.accepted = false
        } else if (editSheet.opened) {
            editSheet.close()
            close.accepted = false
        } else if (stack.depth > 1) {
            stack.pop()
            close.accepted = false
        }
    }

    ExpenseController {
        id: expenseController

        // One hub for live refresh: every expense change updates the list,
        // the dashboard and the budget spends.
        function refreshAll() {
            expensesModel.refresh()
            dashboardModel.refresh()
            budgetsModel.refresh()
        }

        onExpenseAdded: refreshAll()
        onExpenseUpdated: refreshAll()
        onExpenseRemoved: refreshAll()
    }

    ExpenseListModel { id: expensesModel }
    DashboardViewModel { id: dashboardModel }
    BudgetViewModel { id: budgetsModel }
    CategoryListModel { id: categoriesModel }

    BillController {
        id: billController
        // Paying a bill logs an expense, so the expense-side views must refresh too.
        onBillPaid: expenseController.refreshAll()
    }
    BillListModel { id: billsModel }

    Connections {
        target: AppBackend
        // A restore swaps the database file out from under every model.
        function onModelsRefreshNeeded() {
            expensesModel.refresh()
            dashboardModel.refresh()
            budgetsModel.refresh()
            categoriesModel.refresh()
            billsModel.refresh()
        }
    }

    Connections {
        target: budgetsModel
        // Budget edits move the dashboard gauge
        function onOverallChanged() { dashboardModel.refresh() }
    }

    Connections {
        target: ThemeController
        // Category colors re-resolve against the new palette mode
        function onDarkChanged() {
            expensesModel.refresh()
            categoriesModel.refresh()
            budgetsModel.refresh()
            dashboardModel.refresh()
        }
    }

    Connections {
        target: categoriesModel
        // Renames/recolors show up in expense rows and budgets
        function onCountChanged() { expenseController.refreshAll() }
    }

    AddExpenseSheet {
        id: addSheet
        controller: expenseController
    }

    EditExpenseSheet {
        id: editSheet
        controller: expenseController
    }

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: shellPage

        // Mirror the whole UI for right-to-left languages
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true
    }

    Component {
        id: shellPage

        Page {
            background: Rectangle { color: Theme.surface }

            StackLayout {
                anchors.fill: parent
                currentIndex: navBar.currentIndex

                HomeScreen {
                    dashboard: dashboardModel
                    expenses: expensesModel
                    onEditRequested: (expenseId) => editSheet.openFor(expenseId)
                    onReportsRequested: stack.push(reportsPage)
                }
                ExpensesScreen {
                    controller: expenseController
                    model: expensesModel
                    onEditRequested: (expenseId) => editSheet.openFor(expenseId)
                }
                BillsScreen {
                    controller: billController
                    model: billsModel
                }
                BudgetsScreen {
                    budgets: budgetsModel
                }
                SettingsScreen {
                    onManageCategoriesRequested: stack.push(categoryManagerPage)
                }
            }

            footer: NavBar {
                id: navBar
            }

            RoundButton {
                id: fab

                anchors {
                    right: parent.right
                    bottom: parent.bottom
                    margins: Theme.spacingL
                }
                visible: navBar.currentIndex <= 1
                width: 60
                height: 60
                icon.source: "icons/plus.svg"
                icon.width: 26
                icon.height: 26
                Material.background: Theme.accent
                Material.foreground: "#ffffff"
                onClicked: addSheet.open()
            }
        }
    }

    Component {
        id: categoryManagerPage

        CategoryManagerScreen {
            categories: categoriesModel
            onClosed: stack.pop()
        }
    }

    Component {
        id: reportsPage

        Page {
            background: Rectangle { color: Theme.surface }

            ReportsScreen {
                anchors.fill: parent
            }

            header: ToolBar {
                Material.background: Theme.cardColor
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: Theme.spacingM
                    ToolButton {
                        icon.source: "icons/chevron-left.svg"
                        onClicked: stack.pop()
                    }
                    Text {
                        text: qsTr("Reports")
                        font.weight: Font.DemiBold
                        color: Theme.primaryInk
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
