import QtQuick
import Masareef

// Small caption above a form field or a screen section. Exists so the
// caption style is defined once instead of being retyped with slightly
// different sizes and colors on every screen.
Text {
    font.pixelSize: Theme.fontSizeCaption
    font.weight: Font.DemiBold
    font.letterSpacing: 0.5
    color: Theme.mutedInk
    elide: Text.ElideRight
}
