pragma Singleton
import QtQuick
import Masareef

// Central design tokens for the mobile UI. Colors come from the C++
// ThemeController (which owns the Palette mode); spacing and type sizes
// live here so every screen stays on the same grid.
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
    readonly property int fontSizeDisplay: 32

    // Corner radii
    readonly property int radiusS: 8
    readonly property int radiusM: 12
    readonly property int radiusL: 20

    // Minimum touch target (dp)
    readonly property int touchTarget: 48

    // Brand colors (Material primary/accent, matching the app palette)
    readonly property color primary: "#2a78d6"
    readonly property color accent: "#1baf7a"

    readonly property bool dark: ThemeController.dark
    readonly property color surface: ThemeController.surface
    readonly property color cardColor: dark ? "#242423" : "#ffffff"
    readonly property color primaryInk: ThemeController.primaryInk
    readonly property color secondaryInk: ThemeController.secondaryInk
    readonly property color mutedInk: ThemeController.mutedInk
    readonly property color gridline: ThemeController.gridline
    readonly property color good: ThemeController.good
    readonly property color serious: ThemeController.serious
    readonly property color critical: ThemeController.critical
}
