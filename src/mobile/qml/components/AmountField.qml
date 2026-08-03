import QtQuick
import QtQuick.Controls.Material
import Masareef

// Amount entry: numeric keypad, the active currency as a suffix, and live
// validation through the shared money parser.
//
// The label says "Amount", not "0.00". The Material style animates
// placeholderText up into a floating label as soon as the field has focus or
// content, so a sample-value placeholder ends up hovering directly above
// whatever is being typed — which is exactly what it looked like.
TextField {
    id: field

    // Accepts 0 as a real answer. An expense of nothing is a mistake, but
    // "I have not priced this yet" is a useful row in the price book.
    property bool allowZero: false

    // Parsed minor units, or -1 while the text is not a valid amount.
    // `real` rather than `int`: minor units run past 2^31 well before an
    // amount stops being plausible.
    readonly property real amountMinor: AppBackend.parseMoney(text)
    // An empty field parses to -1, so it stays invalid either way.
    readonly property bool valid: allowZero ? amountMinor >= 0 : amountMinor > 0
    // Blank means "not filled in yet", not "wrong" — only complain once
    // there is something in the field that cannot be an amount.
    readonly property bool showsError: text.length > 0 && amountMinor < 0

    // Room for the currency suffix on whichever side is the trailing one.
    readonly property real suffixSpace: currencyLabel.implicitWidth + 2 * Theme.spacingM

    placeholderText: qsTr("Amount")
    inputMethodHints: Qt.ImhFormattedNumbersOnly
    // Accepts Arabic-Indic and Extended Arabic-Indic digits alongside ASCII,
    // and both decimal separators, so an Arabic keypad types straight into
    // it. CurrencyFormatter::parse folds them all back to ASCII.
    validator: RegularExpressionValidator {
        // ٠-٩ Arabic-Indic, ۰-۹ Extended Arabic-Indic,
        // ٫ the Arabic decimal separator.
        regularExpression: /^[0-9٠-٩۰-۹]{0,12}([.,٫][0-9٠-٩۰-۹]{0,2})?$/
    }

    font.pixelSize: Theme.fontSizeTitle
    implicitHeight: Theme.fieldHeight + 8
    // Padding is logical, not visual: Qt Quick Controls do not swap
    // left/rightPadding under mirroring, so the swap is explicit here.
    leftPadding: Theme.rtl ? suffixSpace : Theme.spacingM
    rightPadding: Theme.rtl ? Theme.spacingM : suffixSpace
    Material.accent: showsError ? Theme.critical : Theme.primary

    Text {
        id: currencyLabel

        // Anchors *do* mirror, so this lands opposite the padding reserved
        // for it above in both directions.
        anchors.right: parent.right
        anchors.rightMargin: Theme.spacingM
        anchors.verticalCenter: parent.verticalCenter
        text: AppBackend.currencyCode
        font.pixelSize: Theme.fontSizeSubtitle
        color: Theme.mutedInk
    }
}
