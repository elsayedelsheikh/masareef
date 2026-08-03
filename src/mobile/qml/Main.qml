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

    // Every sheet the back button has to dismiss before it may leave the
    // app. Listed once so adding a sheet cannot silently forget to handle
    // back — which on a phone means the app exiting mid-entry.
    readonly property var sheets: [addSheet, editSheet, addBillSheet,
                                   editBillSheet, addPriceSheet, editPriceSheet]

    // Android back button: close sheets / leave sub-pages before exiting
    onClosing: (close) => {
        for (const sheet of window.sheets) {
            if (sheet.opened) {
                sheet.close()
                close.accepted = false
                return
            }
        }
        if (stack.depth > 1) {
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

    PriceItemController { id: priceItemController }
    PriceListModel { id: pricesModel }

    Connections {
        target: AppBackend
        // A restore swaps the database file out from under every model.
        function onModelsRefreshNeeded() {
            expensesModel.refresh()
            dashboardModel.refresh()
            budgetsModel.refresh()
            categoriesModel.refresh()
            billsModel.refresh()
            pricesModel.refresh()
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
            billsModel.refresh()
            pricesModel.refresh()
        }
    }

    Connections {
        target: AppBackend
        // Every model formats its own dates and amounts, so a language
        // switch has to go back through them — a retranslate only reaches
        // the qsTr() strings in QML.
        function onLanguageChanged() {
            expensesModel.refresh()
            dashboardModel.refresh()
            budgetsModel.refresh()
            billsModel.refresh()
            pricesModel.refresh()
        }
    }

    Connections {
        target: categoriesModel
        // Renames/recolors show up in expense rows, budgets and prices
        function onCountChanged() {
            expenseController.refreshAll()
            billsModel.refresh()
            pricesModel.refresh()
        }
    }

    AddExpenseSheet {
        id: addSheet
        controller: expenseController
    }

    EditExpenseSheet {
        id: editSheet
        controller: expenseController
    }

    AddBillSheet {
        id: addBillSheet
        controller: billController
    }

    EditBillSheet {
        id: editBillSheet
        controller: billController
    }

    AddPriceItemSheet {
        id: addPriceSheet
        controller: priceItemController
    }

    EditPriceItemSheet {
        id: editPriceSheet
        controller: priceItemController
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

            Connections {
                target: editPriceSheet
                // The sheet never opened, so its own banner would never be
                // seen — the message belongs back on the screen that asked
                // for it.
                function onOpenFailed(message) {
                    priceBookScreen.errorMessage = message
                }
            }

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
                    onEditRequested: (billId) => editBillSheet.openFor(billId)
                }
                PriceBookScreen {
                    id: priceBookScreen
                    controller: priceItemController
                    model: pricesModel
                    onEditRequested: (priceItemId, previousPrice) =>
                        editPriceSheet.openFor(priceItemId, previousPrice)
                    onLogExpenseRequested: (categoryId, amountText, description) =>
                        addSheet.openPrefilled(categoryId, amountText, description)
                    onDuplicateRequested: (name, categoryId, unit, priceText) =>
                        addPriceSheet.openCopyOf(name, categoryId, unit, priceText)
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

                // What "+" means depends on the tab: an expense on
                // Home/Expenses, a bill on Bills, a price on Prices. The
                // tabs with nothing to add hide it rather than adding
                // whatever the previous tab would have.
                readonly property int addsOnTab: navBar.currentIndex

                anchors {
                    right: parent.right
                    bottom: parent.bottom
                    margins: Theme.spacingL
                }
                visible: addsOnTab <= 3
                width: 60
                height: 60
                icon.source: "icons/plus.svg"
                icon.width: 26
                icon.height: 26
                Material.background: Theme.accent
                Material.foreground: "#ffffff"
                onClicked: {
                    switch (addsOnTab) {
                    case 2:
                        addBillSheet.open()
                        break
                    case 3:
                        addPriceSheet.open()
                        break
                    default:
                        addSheet.open()
                        break
                    }
                }
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
