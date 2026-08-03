import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    property int lastEditId: -1

    BillsScreen {
        id: screen
        anchors.fill: parent
        onEditRequested: (billId) => root.lastEditId = billId
    }

    AddBillSheet {
        id: addSheet
        controller: screen.controller
    }

    EditBillSheet {
        id: editSheet
        controller: screen.controller
    }

    TestCase {
        id: testCase
        name: "BillsScreen"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
            screen.showPaused = false // also resets the filter control
            screen.model.refresh() // drop rows cached from a prior test
            root.lastEditId = -1
        }

        function addBill(name, amount, daysFromNow) {
            const catId = TestFixture.categoryId("Bills")
            const due = new Date()
            due.setDate(due.getDate() + daysFromNow)
            const id = screen.controller.add(catId, amount, name, due, 0, "")
            verify(id > 0, screen.controller.lastError)
            return id
        }

        function test_emptyScreenShowsEmptyState() {
            verify(screen.visible)
            compare(screen.model.billIdAt(0), -1)
        }

        // Guards the add path the FAB/AddBillSheet drives through the controller
        // (recurrence passed as the combo index, 0 = Monthly), plus the
        // model-refresh Connections that keep the list in sync.
        function test_addBillAppearsInList() {
            const catId = TestFixture.categoryId("Bills")
            const id = screen.controller.add(catId, "50", "Internet",
                                             new Date(2026, 7, 1), 0, "")
            verify(id > 0)
            tryVerify(function() { return screen.model.billIdAt(0) > 0 })
        }

        // Guards the "Paid" button path: markPaid logs an expense and keeps the
        // (now-advanced) bill in the list.
        function test_markPaidLogsExpenseAndKeepsBill() {
            const catId = TestFixture.categoryId("Bills")
            const id = screen.controller.add(catId, "50", "Internet",
                                             new Date(2026, 6, 1), 0, "")
            tryVerify(function() { return screen.model.billIdAt(0) > 0 })
            compare(TestFixture.expenseCount(), 0)

            verify(screen.controller.markPaid(id, "50", new Date(2026, 6, 15),
                                              "Internet"))
            compare(TestFixture.expenseCount(), 1)
            verify(screen.model.billIdAt(0) > 0)
        }

        // The Paid button and the "Mark paid" action both go through
        // payRow, which sends the amount as exact text rather than a
        // divided float.
        function test_payRowLogsTheExactAmount() {
            addBill("Internet", "1234.56", 3)
            tryVerify(function() { return screen.model.count === 1 })
            compare(TestFixture.expenseCount(), 0)

            screen.payRow(screen._rowAt(0))
            compare(TestFixture.expenseCount(), 1)
        }

        function test_pausingHidesTheBillAndSwitchesTheFilter() {
            addBill("Gym", "300", 5)
            tryVerify(function() { return screen.model.count === 1 })

            screen.setRowActive(screen._rowAt(0), false)
            // The filter follows, so the paused bill stays reachable
            // instead of vanishing the moment it is paused.
            tryVerify(function() { return screen.showPaused })
            compare(screen.model.count, 1)
            compare(screen.model.activeCount, 0)
        }

        function test_resumingBringsTheBillBack() {
            addBill("Gym", "300", 5)
            tryVerify(function() { return screen.model.count === 1 })

            screen.setRowActive(screen._rowAt(0), false)
            tryVerify(function() { return screen.model.activeCount === 0 })

            screen.setRowActive(screen._rowAt(0), true)
            tryVerify(function() { return screen.model.activeCount === 1 })
        }

        function test_monthlyTotalCountsOnlyActiveBills() {
            addBill("Gym", "300", 5)
            addBill("Internet", "200", 6)
            tryVerify(function() { return screen.model.count === 2 })
            const both = screen.model.monthlyTotalFormatted

            screen.setRowActive(screen._rowAt(0), false)
            tryVerify(function() {
                return screen.model.monthlyTotalFormatted !== both
            })
            compare(screen.model.activeCount, 1)
        }

        function test_actionSheetEditReportsTheBill() {
            const id = addBill("Gym", "300", 5)
            tryVerify(function() { return screen.model.count === 1 })

            screen.openActionsFor(0)
            screen._runAction("edit")
            compare(root.lastEditId, id)
        }

        function test_actionsOnACapturedRowAreIgnoredWhenThereIsNone() {
            addBill("Gym", "300", 5)
            tryVerify(function() { return screen.model.count === 1 })

            screen._actionRow = null
            screen._runAction("delete")
            screen._deleteActionRow()
            compare(screen.model.count, 1)
        }

        // The segmented control's index is not a Recurrence value; the form
        // maps between them. Saving a bill as Yearly and reopening it has to
        // land on the same segment, whatever order either list is in.
        function test_recurrenceSurvivesTheRoundTripThroughTheSheets() {
            addSheet.startOver()
            addSheet.nameText = "Insurance"
            addSheet.amountText = "1200"
            addSheet.selectedCategoryId = TestFixture.categoryId("Bills")
            addSheet.nextDueDate = new Date(2026, 7, 1)
            addSheet.selectedRecurrence = 2 // the "Yearly" segment
            verify(addSheet.canSave)
            addSheet.save()
            verify(addSheet.errorMessage.length === 0, addSheet.errorMessage)

            tryVerify(function() { return screen.model.count === 1 })
            const id = screen.model.billIdAt(0)

            editSheet.openFor(id)
            tryVerify(function() { return editSheet.opened })
            // Stored as the enum, shown as the segment.
            compare(screen.controller.editRecurrence, Recurrence.Yearly)
            compare(editSheet.selectedRecurrence, 2)
            editSheet.close()
        }

        function test_deleteRemovesTheBill() {
            addBill("Gym", "300", 5)
            tryVerify(function() { return screen.model.count === 1 })

            screen.openActionsFor(0)
            screen._deleteActionRow()
            tryVerify(function() { return screen.model.count === 0 })
        }
    }
}
