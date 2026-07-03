#pragma once

#include <QColor>
#include <QString>

// Validated palette (light and dark selected separately — the dark column
// is the same hues re-stepped for the dark surface, not an automatic flip).
// The categorical slot order is the colorblind-safety mechanism, do not
// reshuffle. Slots 1-3 seed the system categories.
//
// The DB stores the *light* hex per category; series() maps a stored color
// to its dark-mode counterpart at display time, so the data never changes
// with the theme.
namespace Palette {

enum class Mode { Light, Dark };

void setMode(Mode mode); // set by Theme::apply()
[[nodiscard]] Mode mode();

// Stored (light) categorical hex values — what gets written to the DB
inline constexpr const char* kCategorical[] = {
    "#2a78d6", // blue    — Bills
    "#1baf7a", // aqua    — Groceries
    "#eda100", // yellow  — Other
    "#008300", // green
    "#4a3aa7", // violet
    "#e34948", // red
    "#e87ba4", // magenta
    "#eb6834", // orange
};
inline constexpr int kCategoricalCount = 8;

// Display color for a stored category color in the current mode. Known
// categorical slots swap to their dark-mode step; custom user colors pass
// through unchanged.
[[nodiscard]] QColor series(const QString& storedColor);

// Status colors (reserved for state, never used as series colors)
[[nodiscard]] QColor critical(); // overdue / over budget
[[nodiscard]] QColor serious();  // due within 7 days / near budget limit
[[nodiscard]] QColor good();

// Chart chrome & ink
[[nodiscard]] QColor surface();
[[nodiscard]] QColor gridline();
[[nodiscard]] QColor axisLine();
[[nodiscard]] QColor mutedInk();
[[nodiscard]] QColor primaryInk();
[[nodiscard]] QColor secondaryInk();

} // namespace Palette
