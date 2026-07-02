#pragma once

// Validated categorical palette (fixed slot order — the order is the
// colorblind-safety mechanism, do not reshuffle). Slots 1-3 seed the
// system categories; the rest are offered for user categories.
namespace Palette {

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

// Status colors (reserved for state, never used as series colors)
inline constexpr const char* kCritical = "#d03b3b"; // overdue
inline constexpr const char* kSerious  = "#ec835a"; // due within 7 days
inline constexpr const char* kGood     = "#0ca30c";

// Chart chrome
inline constexpr const char* kSurface   = "#fcfcfb";
inline constexpr const char* kGridline  = "#e1e0d9";
inline constexpr const char* kAxisLine  = "#c3c2b7";
inline constexpr const char* kMutedInk  = "#898781";
inline constexpr const char* kPrimaryInk = "#0b0b0b";
inline constexpr const char* kSecondaryInk = "#52514e";

} // namespace Palette
