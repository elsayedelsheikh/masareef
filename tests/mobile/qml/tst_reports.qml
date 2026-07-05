import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    ReportsScreen {
        id: screen
        anchors.fill: parent
    }

    TestCase {
        id: testCase
        name: "ReportsScreen"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
        }

        // Guards the gadget property exposure that the chart bindings read.
        // A plain-struct model returns undefined here and the screen renders blank.
        function test_monthlyTotalsExposeProperties() {
            TestFixture.addExpense("Groceries", 2000, "test", "2026-06-15")
            const totals = screen.model.monthlyTotals()
            verify(totals.length > 0)
            verify(totals[0].monthName.length > 0)
            verify(totals[0].totalFormatted.length > 0)
            // monthlyTotals returns all 12 months incl. zeros; find the funded one
            let funded = 0
            for (let i = 0; i < totals.length; i++)
                if (totals[i].totalMinor > 0) funded++
            compare(funded, 1)
        }

        function findByName(item, name) {
            if (!item)
                return null
            if (item.objectName === name)
                return item
            const kids = item.children
            for (let i = 0; kids && i < kids.length; i++) {
                const r = findByName(kids[i], name)
                if (r)
                    return r
            }
            return null
        }

        // Regression: the category rows once rendered with zero width/height
        // (Column-in-ScrollView never laid them out) so the screen looked blank.
        // A fresh screen with current-month data must produce a sized delegate.
        function test_categoryRowsRender() {
            const today = new Date()
            TestFixture.addExpense("Bills", 3500, "b",
                Qt.formatDate(new Date(today.getFullYear(), today.getMonth(), 4), "yyyy-MM-dd"))
            const fresh = Qt.createQmlObject(
                'import Masareef; ReportsScreen { anchors.fill: parent }', root)
            const list = findByName(fresh, "categoryList")
            verify(list !== null)
            tryVerify(function() { return list.count === 1 })
            const row = list.itemAtIndex(0)
            verify(row !== null)
            verify(row.width > 0)
            verify(row.height > 0)
            fresh.destroy()
        }

        function test_categoryTotalsExposeProperties() {
            TestFixture.addExpense("Bills", 1000, "test", "2026-06-15")
            const totals = screen.model.categoryTotals(
                new Date(2026, 5, 1), new Date(2026, 5, 30))
            verify(totals.length > 0)
            verify(totals[0].categoryName.length > 0)
            verify(totals[0].totalFormatted.length > 0)
            verify(String(totals[0].categoryColor).length > 0)
        }
    }
}
