import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    AddExpenseSheet {
        id: sheet
        controller: expenseController
    }

    ExpenseController {
        id: expenseController
    }

    SignalSpy {
        id: addedSpy
        target: expenseController
        signalName: "expenseAdded"
    }

    TestCase {
        id: testCase
        name: "AddExpenseSheet"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
            addedSpy.clear()
            sheet.open()
            tryVerify(function() { return sheet.opened })
        }

        function cleanup() {
            sheet.close()
            tryVerify(function() { return !sheet.opened })
        }

        function test_saveDisabledUntilFormValid() {
            verify(!sheet.canSave, "empty form must not be savable")

            sheet.amountText = "abc"
            sheet.selectedCategoryId = TestFixture.categoryId("Bills")
            verify(!sheet.canSave, "unparseable amount must not be savable")

            sheet.amountText = "0"
            verify(!sheet.canSave, "zero amount must not be savable")

            sheet.amountText = "120.50"
            verify(sheet.canSave, "valid amount + category must be savable")

            sheet.selectedCategoryId = -1
            verify(!sheet.canSave, "no category must not be savable")
        }

        function test_saveAddsExpenseAndCloses() {
            sheet.amountText = "250.99"
            sheet.selectedCategoryId = TestFixture.categoryId("Groceries")
            sheet.descriptionText = "Bread"
            verify(sheet.canSave)

            sheet.save()
            compare(addedSpy.count, 1)
            compare(TestFixture.expenseCount(), 1)
            tryVerify(function() { return !sheet.opened })
        }

        function test_amountFieldUsesNumericKeyboard() {
            verify(sheet.amountInputHints & Qt.ImhFormattedNumbersOnly,
                   "amount field must request the numeric keyboard")
        }

        function test_reopenStartsClean() {
            sheet.amountText = "10"
            sheet.descriptionText = "leftover"
            sheet.close()
            tryVerify(function() { return !sheet.opened })
            sheet.open()
            tryVerify(function() { return sheet.opened })
            compare(sheet.amountText, "")
            compare(sheet.descriptionText, "")
        }
    }
}
