import QtQuick
import QtQuick.Controls.impl as Impl
import Masareef

// A tinted icon outside of a button.
//
// The icons are black stroke SVGs, so anything showing one on a dark
// surface has to recolor it. Controls do that through `icon.color`; a bare
// Image cannot. IconImage is the same class the controls use internally —
// it lives in a private import, which is why every use in the app goes
// through this one file rather than importing it in a dozen places.
//
// `iconName` is a bare icon name ("pencil"), resolved here against qml/icons
// so callers never have to get a relative path right from their own
// directory. It is not called `name`: IconImage already has a FINAL `name`
// for icon-theme lookup, and shadowing it is a load-time error.
Impl.IconImage {
    id: icon

    property string iconName
    property int size: 22

    source: iconName.length > 0
        ? Qt.resolvedUrl("../icons/" + iconName + ".svg") : ""
    sourceSize: Qt.size(size, size)
    color: Theme.secondaryInk
}
