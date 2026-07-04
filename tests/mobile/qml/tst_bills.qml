import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    BillsScreen {
        id: screen
        anchors.fill: parent
    }

    TestCase {
        id: testCase
        name: "BillsScreen"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
            screen.model.refresh() // drop rows cached from a prior test
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
    }
}
