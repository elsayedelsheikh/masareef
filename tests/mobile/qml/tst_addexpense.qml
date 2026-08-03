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
            // The sheet deliberately keeps its draft across a close, so each
            // test has to start it over explicitly.
            sheet.startOver()
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

        // Closing the sheet must not throw the entry away. An interruption
        // — the back button, a mis-tap, switching apps — leaves the
        // half-filled form intact, and reopening says so rather than
        // silently resuming.
        function test_reopenResumesTheDraft() {
            sheet.amountText = "10"
            sheet.descriptionText = "leftover"
            sheet.close()
            tryVerify(function() { return !sheet.opened })

            sheet.open()
            tryVerify(function() { return sheet.opened })
            compare(sheet.amountText, "10")
            compare(sheet.descriptionText, "leftover")
            verify(sheet.resumedDraft, "a resumed draft must announce itself")
        }

        function test_startOverClearsTheDraft() {
            sheet.amountText = "10"
            sheet.descriptionText = "leftover"
            sheet.selectedCategoryId = TestFixture.categoryId("Bills")

            sheet.startOver()
            compare(sheet.amountText, "")
            compare(sheet.descriptionText, "")
            compare(sheet.selectedCategoryId, -1)
            verify(!sheet.resumedDraft)
        }

        // Cancel is the deliberate "throw this away", unlike a close.
        function test_cancelClearsTheDraft() {
            sheet.amountText = "10"
            sheet.descriptionText = "leftover"

            sheet.discard()
            tryVerify(function() { return !sheet.opened })
            sheet.open()
            tryVerify(function() { return sheet.opened })
            compare(sheet.amountText, "")
            verify(!sheet.resumedDraft)
        }

        function test_saveClearsTheDraft() {
            sheet.amountText = "250.99"
            sheet.selectedCategoryId = TestFixture.categoryId("Groceries")
            sheet.descriptionText = "Bread"
            sheet.save()
            tryVerify(function() { return !sheet.opened })

            sheet.open()
            tryVerify(function() { return sheet.opened })
            compare(sheet.amountText, "")
            compare(sheet.descriptionText, "")
            verify(!sheet.resumedDraft)
        }

        // A failed save keeps everything, so nothing has to be retyped.
        function test_failedSaveKeepsTheForm() {
            sheet.amountText = "250.99"
            sheet.descriptionText = "Bread"
            // No category, so the controller refuses it.
            sheet.selectedCategoryId = -1
            sheet.save()

            compare(addedSpy.count, 0)
            compare(sheet.amountText, "250.99")
            compare(sheet.descriptionText, "Bread")
        }

        // Logging a price book item lands here prefilled, and says so
        // instead of looking like a resumed draft.
        function test_openPrefilledFillsTheFormAndLabelsItself() {
            const catId = TestFixture.categoryId("Groceries")
            sheet.openPrefilled(catId, "45.50", "Milk")
            tryVerify(function() { return sheet.opened })

            compare(sheet.amountText, "45.50")
            compare(sheet.descriptionText, "Milk")
            compare(sheet.selectedCategoryId, catId)
            verify(sheet.prefilled)
            verify(!sheet.resumedDraft)
            verify(sheet.canSave)
        }
    }
}
