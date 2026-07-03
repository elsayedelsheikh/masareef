import QtQuick
import QtTest
import Masareef

Item {
    id: root
    width: 420
    height: 900

    NavBar {
        id: navBar
        width: parent.width
        anchors.bottom: parent.bottom
    }

    TestCase {
        id: testCase
        name: "NavBar"
        when: windowShown

        function test_hasFourTabs() {
            compare(navBar.count, 4)
        }

        function test_clickSwitchesTab() {
            navBar.currentIndex = 0
            const tabWidth = navBar.width / navBar.count
            // Tap the center of the third tab
            mouseClick(navBar, tabWidth * 2.5, navBar.height / 2)
            compare(navBar.currentIndex, 2)
            mouseClick(navBar, tabWidth * 0.5, navBar.height / 2)
            compare(navBar.currentIndex, 0)
        }

        function test_languageSwitchRetranslatesLive() {
            compare(navBar.itemAt(0).text, "Home")

            AppBackend.language = "ar"
            try {
                tryCompare(navBar.itemAt(0), "text", "الرئيسية")
            } finally {
                AppBackend.language = "en"
            }
            tryCompare(navBar.itemAt(0), "text", "Home")
        }
    }
}
