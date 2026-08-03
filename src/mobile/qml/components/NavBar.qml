import QtQuick
import QtQuick.Controls.Material
import Masareef

// Bottom navigation: Home, Expenses, Bills, Prices, Budgets, Settings.
//
// TabBar divides its width evenly between the tabs, so six of them get
// about 68dp each on a 412dp phone — enough for "Expenses" and for the
// longer Arabic labels at the caption size, but not with much to spare.
// On a narrower screen the labels drop a step rather than run together.
TabBar {
    id: bar

    // Below this, six labels at the normal caption size stop fitting.
    readonly property bool tight: width > 0 && width / 6 < 66

    position: TabBar.Footer
    Material.background: Theme.cardColor

    component NavButton: TabButton {
        display: AbstractButton.TextUnderIcon
        icon.width: 22
        icon.height: 22
        font.pixelSize: bar.tight ? Theme.fontSizeCaption - 1
                                  : Theme.fontSizeCaption
        implicitHeight: Theme.touchTarget + 16
    }

    NavButton {
        text: qsTr("Home")
        icon.source: "../icons/home.svg"
    }
    NavButton {
        text: qsTr("Expenses")
        icon.source: "../icons/receipt.svg"
    }
    NavButton {
        text: qsTr("Bills")
        icon.source: "../icons/bell.svg"
    }
    NavButton {
        text: qsTr("Prices")
        icon.source: "../icons/basket.svg"
    }
    NavButton {
        text: qsTr("Budgets")
        icon.source: "../icons/donut.svg"
    }
    NavButton {
        text: qsTr("Settings")
        icon.source: "../icons/gear.svg"
    }
}
