import QtQuick
import QtQuick.Controls.Material
import Masareef

// Bottom navigation: Home, Expenses, Budgets, Settings.
TabBar {
    id: bar

    position: TabBar.Footer
    Material.background: Theme.cardColor

    component NavButton: TabButton {
        display: AbstractButton.TextUnderIcon
        icon.width: 22
        icon.height: 22
        font.pixelSize: Theme.fontSizeCaption
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
        text: qsTr("Budgets")
        icon.source: "../icons/donut.svg"
    }
    NavButton {
        text: qsTr("Settings")
        icon.source: "../icons/gear.svg"
    }
}
