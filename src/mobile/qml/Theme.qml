pragma Singleton
import QtQuick
import Masareef

// Central design tokens for the mobile UI. Colors come from the C++
// ThemeController (which owns the Palette mode); spacing, type sizes,
// motion and elevation live here so every screen stays on the same grid.
QtObject {
    // Spacing scale (dp)
    readonly property int spacingXs: 4
    readonly property int spacingS: 8
    readonly property int spacingM: 16
    readonly property int spacingL: 24
    readonly property int spacingXl: 32

    // Type scale (sp)
    readonly property int fontSizeCaption: 12
    readonly property int fontSizeBody: 14
    readonly property int fontSizeSubtitle: 16
    readonly property int fontSizeTitle: 20
    readonly property int fontSizeHeadline: 24
    readonly property int fontSizeDisplay: 32

    // Corner radii
    readonly property int radiusS: 8
    readonly property int radiusM: 12
    readonly property int radiusL: 20
    readonly property int radiusXl: 28

    // Minimum touch target (dp)
    readonly property int touchTarget: 48
    // Text fields need more than the touch target: the Material style lifts
    // the placeholder into a floating label above the text, and 48dp clips it.
    readonly property int fieldHeight: 56

    // Motion. One place to slow everything down or speed it up; the easing
    // curve is deliberately the same everywhere so the app feels like one
    // surface rather than a pile of screens.
    readonly property int durationFast: 120
    readonly property int durationMedium: 220
    readonly property int durationSlow: 320
    readonly property int easing: Easing.OutCubic

    // True when the UI is running right-to-left. Items that need to know
    // read this instead of Qt.application.layoutDirection, and containers
    // that hold popups mirror on it (a Popup is not an Item, so the
    // LayoutMirroring attached property has to go on its content root).
    readonly property bool rtl: Qt.application.layoutDirection === Qt.RightToLeft

    // Brand colors (Material primary/accent, matching the app palette)
    readonly property color primary: "#2a78d6"
    readonly property color accent: "#1baf7a"
    // Text and icons drawn *on* a primary or accent fill. Both brand colors
    // are fixed across light and dark, so this one is fixed too — it is not
    // the theme's ink, which would vanish into a blue chip in dark mode.
    // Not named onPrimary: QML reads a leading "on" as a signal handler.
    readonly property color inkOnPrimary: "#ffffff"

    readonly property bool dark: ThemeController.dark
    readonly property color surface: ThemeController.surface
    readonly property color cardColor: dark ? "#242423" : "#ffffff"
    // One step above a card: sheets, menus and anything that floats
    readonly property color elevatedColor: dark ? "#2e2e2c" : "#ffffff"
    readonly property color primaryInk: ThemeController.primaryInk
    readonly property color secondaryInk: ThemeController.secondaryInk
    readonly property color mutedInk: ThemeController.mutedInk
    readonly property color gridline: ThemeController.gridline
    readonly property color good: ThemeController.good
    readonly property color serious: ThemeController.serious
    readonly property color critical: ThemeController.critical

    // Subtle fills derived from the brand and status colors, for chips,
    // selected rows and tinted banners.
    readonly property color primaryTint: Qt.alpha(primary, dark ? 0.24 : 0.12)
    readonly property color accentTint: Qt.alpha(accent, dark ? 0.24 : 0.12)
    readonly property color criticalTint: Qt.alpha(critical, dark ? 0.26 : 0.12)
    readonly property color seriousTint: Qt.alpha(serious, dark ? 0.26 : 0.14)
    readonly property color goodTint: Qt.alpha(good, dark ? 0.26 : 0.12)
    readonly property color pressedTint: Qt.alpha(primaryInk, 0.08)

    // Scrim behind modal surfaces
    readonly property color scrim: Qt.alpha("#000000", dark ? 0.6 : 0.4)
}
