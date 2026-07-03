#pragma once

// Applies the configured theme (AppConfig::theme) to the whole
// application: widget style/palette and the chart palette mode.
namespace Theme {

// Reads the preference, resolves System against the OS color scheme and
// restyles the application. Call at startup and after Settings changes.
void apply();

[[nodiscard]] bool isDark();

} // namespace Theme
