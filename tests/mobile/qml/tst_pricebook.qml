import QtQuick
import QtTest
import Masareef

// Drives the price book screen the way the shell does: the screen owns no
// data of its own, it reads a PriceListModel and pushes every change
// through a PriceItemController, and everything that opens a sheet leaves
// as a signal.
Item {
    id: root
    width: 420
    height: 900

    property int lastEditId: -1
    property string lastEditPreviousPrice: ""
    property var lastLogged: null
    property var lastDuplicated: null

    PriceBookScreen {
        id: screen
        anchors.fill: parent

        onEditRequested: (priceItemId, previousPrice) => {
            root.lastEditId = priceItemId
            root.lastEditPreviousPrice = previousPrice
        }
        onLogExpenseRequested: (categoryId, amountText, description) => {
            root.lastLogged = { categoryId, amountText, description }
        }
        onDuplicateRequested: (name, categoryId, unit, priceText) => {
            root.lastDuplicated = { name, categoryId, unit, priceText }
        }
    }

    TestCase {
        id: testCase
        name: "PriceBookScreen"
        when: windowShown

        function init() {
            verify(TestFixture.resetDatabase())
            screen.model.searchText = ""
            screen.model.refresh() // drop rows cached from a prior test
            screen.errorMessage = ""
            root.lastEditId = -1
            root.lastEditPreviousPrice = ""
            root.lastLogged = null
            root.lastDuplicated = null
        }

        function addItem(name, price, unit) {
            const catId = TestFixture.categoryId("Groceries")
            const id = screen.controller.add(name, catId, unit === undefined ? "kg" : unit,
                                             price)
            verify(id > 0, screen.controller.lastError)
            return id
        }

        function test_emptyScreenHasNoRows() {
            verify(screen.visible)
            compare(screen.model.count, 0)
            compare(screen.model.priceItemIdAt(0), -1)
        }

        // Guards the FAB/AddPriceItemSheet path plus the Connections that
        // keep the list in sync with the controller.
        function test_addedItemAppearsInList() {
            const id = addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })
            compare(screen.model.priceItemIdAt(0), id)
        }

        function test_searchFiltersTheList() {
            addItem("Milk", "45.50", "litre")
            addItem("Rice", "30", "kg")
            tryVerify(function() { return screen.model.count === 2 })

            screen.model.searchText = "mil"
            compare(screen.model.count, 1)

            screen.model.searchText = ""
            compare(screen.model.count, 2)
        }

        function test_tapToEditReportsTheRow() {
            const id = addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })

            screen.openEditFor(0)
            compare(root.lastEditId, id)
            // No price history beyond the first entry yet, so nothing to
            // compare against.
            compare(root.lastEditPreviousPrice, "")
        }

        function test_editCarriesThePreviousPriceOnceItMoves() {
            const id = addItem("Milk", "40", "litre")
            tryVerify(function() { return screen.model.count === 1 })
            verify(screen.controller.update(id, "Milk",
                                            TestFixture.categoryId("Groceries"),
                                            "litre", "50"))
            tryVerify(function() { return screen.model.count === 1 })

            screen.openEditFor(0)
            compare(root.lastEditId, id)
            verify(root.lastEditPreviousPrice.length > 0)
        }

        function test_openEditForAnOutOfRangeRowDoesNothing() {
            screen.openEditFor(4)
            compare(root.lastEditId, -1)
        }

        function test_logAsExpenseCarriesThePriceAndName() {
            addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })

            screen.openActionsFor(0)
            screen._runAction("log")

            verify(root.lastLogged !== null)
            compare(root.lastLogged.description, "Milk")
            compare(root.lastLogged.categoryId, TestFixture.categoryId("Groceries"))
            // Ungrouped, so the amount field can parse it straight back.
            // The exact spelling is locale-independent by construction:
            // LocaleFormat::amountForEditing always writes ASCII digits and
            // a "." separator, whatever the UI language is. If that ever
            // becomes locale-aware, this is the assertion that says so.
            compare(root.lastLogged.amountText, "45.50")
        }

        function test_duplicateCarriesADistinctName() {
            addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })

            screen.openActionsFor(0)
            screen._runAction("duplicate")

            verify(root.lastDuplicated !== null)
            // The price book keys on name, so a straight copy would collide.
            verify(root.lastDuplicated.name !== "Milk")
            verify(root.lastDuplicated.name.indexOf("Milk") === 0)
            compare(root.lastDuplicated.unit, "litre")
            compare(root.lastDuplicated.priceText, "45.50")
        }

        function test_deleteRemovesTheRow() {
            addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })

            screen.openActionsFor(0)
            screen._deleteActionRow()
            tryVerify(function() { return screen.model.count === 0 })
        }

        // A delete that cannot happen has to say why: the row was captured
        // when the sheet opened, and it can be gone by the time the
        // confirmation comes back.
        function test_aFailedDeleteReportsItself() {
            const id = addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })

            screen.openActionsFor(0)
            verify(screen.controller.remove(id)) // deleted from elsewhere
            screen._deleteActionRow()

            verify(screen.errorMessage.length > 0,
                   "the failure must reach the screen")
        }

        function test_actionsOnAnOutOfRangeRowAreIgnored() {
            addItem("Milk", "45.50", "litre")
            tryVerify(function() { return screen.model.count === 1 })

            // Nothing captured, so nothing must happen — in particular the
            // stale row from a previous open must not be acted on.
            screen._actionRow = null
            screen._runAction("edit")
            screen._runAction("log")
            screen._runAction("duplicate")
            screen._runAction("delete")
            screen._deleteActionRow()

            compare(screen.model.count, 1)
            compare(root.lastEditId, -1)
            compare(root.lastLogged, null)
            compare(root.lastDuplicated, null)
            compare(screen.errorMessage, "")
        }
    }
}
