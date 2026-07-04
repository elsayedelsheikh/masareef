import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    CategoryManagerScreen {
        id: screen
        anchors.fill: parent
        categories: categoryModel
    }

    CategoryListModel {
        id: categoryModel
    }

    TestCase {
        id: testCase
        name: "CategoryManagerScreen"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
            categoryModel.refresh()
        }

        function test_deleteUnusedCategoryRemovesIt() {
            verify(categoryModel.add("Transport", "#008300"))
            const id = TestFixture.categoryId("Transport")

            screen.requestDelete(id, "Transport", false)

            compare(TestFixture.categoryId("Transport"), -1)
        }

        function test_deleteInUseCategoryOpensReassignDialog() {
            const groceriesId = TestFixture.categoryId("Groceries")
            verify(TestFixture.addExpense("Groceries", 500, "Bread", "2026-06-10") > 0)

            screen.requestDelete(groceriesId, "Groceries", true)

            tryVerify(function() { return screen.reassignDialog.opened })
        }

        function test_confirmReassignMovesExpenseAndDeletesCategory() {
            const groceriesId = TestFixture.categoryId("Groceries")
            const billsId = TestFixture.categoryId("Bills")
            const expenseId = TestFixture.addExpense("Groceries", 500, "Bread",
                                                      "2026-06-10")
            verify(expenseId > 0)

            screen.requestDelete(groceriesId, "Groceries", true)
            tryVerify(function() { return screen.reassignDialog.opened })

            screen.reassignPicker.selectedCategoryId = billsId
            screen.reassignDialog.accept()

            compare(TestFixture.categoryId("Groceries"), -1)
            compare(TestFixture.expenseCategoryId(expenseId), billsId)
        }

        function test_reassignDialogExcludesTheCategoryBeingDeleted() {
            const groceriesId = TestFixture.categoryId("Groceries")
            verify(TestFixture.addExpense("Groceries", 500, "Bread", "2026-06-10") > 0)

            screen.requestDelete(groceriesId, "Groceries", true)

            compare(screen.reassignPicker.excludeCategoryId, groceriesId)
        }
    }
}
