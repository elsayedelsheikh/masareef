import QtQuick
import QtQuick.Controls.Material
import Masareef

// Amount entry with the numeric keyboard and live validation through the
// shared money parser.
TextField {
    id: field

    // Parsed minor units, or -1 while the text is not a valid amount
    readonly property int amountMinor: AppBackend.parseMoney(text)
    readonly property bool valid: amountMinor > 0

    inputMethodHints: Qt.ImhFormattedNumbersOnly
    placeholderText: qsTr("0.00")
    font.pixelSize: Theme.fontSizeTitle
    implicitHeight: Theme.touchTarget + 8
}
