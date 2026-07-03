import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    ExpensesScreen {
        id: screen
        anchors.fill: parent
        controller: expenseController
        model: listModel
    }

    ExpenseController {
        id: expenseController
    }

    ExpenseListModel {
        id: listModel
    }

    TestCase {
        id: testCase
        name: "ExpensesScreen"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
            verify(TestFixture.addExpense("Bills", 10000, "Electricity",
                                          "2026-06-15") > 0)
            verify(TestFixture.addExpense("Groceries", 5000, "Bread",
                                          "2026-06-10") > 0)
            screen.searchText = ""
            listModel.searchText = ""
            listModel.categoryId = -1
            listModel.refresh()
        }

        function test_listShowsExpenses() {
            compare(listModel.count, 2)
        }

        function test_searchFiltersList() {
            screen.searchText = "Elect"
            tryCompare(listModel, "count", 1)
            screen.searchText = ""
            tryCompare(listModel, "count", 2)
        }

        function test_removeDeletesWithUndo() {
            screen.removeAt(0)
            compare(listModel.count, 1)
            compare(TestFixture.expenseCount(), 1)
            verify(screen.undoVisible, "undo snackbar must appear after removal")

            screen.undo()
            tryCompare(listModel, "count", 2)
            compare(TestFixture.expenseCount(), 2)
            // Popup.close() animates out; visible stays true during the
            // exit transition
            tryVerify(() => !screen.undoVisible)
        }

        function test_editRequestEmitsExpenseId() {
            let requestedId = -1
            function onEdit(id) { requestedId = id }
            screen.editRequested.connect(onEdit)
            screen.requestEditAt(0)
            screen.editRequested.disconnect(onEdit)
            verify(requestedId > 0)
            compare(requestedId, listModel.expenseIdAt(0))
        }
    }
}
