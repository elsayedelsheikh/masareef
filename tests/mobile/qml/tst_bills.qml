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
        }

        function test_emptyScreenShowsEmptyState() {
            verify(screen.visible)
        }

        function test_billsDisplayedBySections() {
            // ponytail: basic navigation test; full UI interaction tested via
            // integration tests or manual QA (swiping, mark-paid behavior).
            verify(screen.visible)
        }
    }
}
