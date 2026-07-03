#pragma once

#include <QString>

enum class ThemePreference { System, Light, Dark };

// QSettings-backed application configuration.
namespace AppConfig {

// ISO currency code shown after every amount, e.g. "100.50 EGP".
[[nodiscard]] QString currencyCode(); // default "EGP"
void setCurrencyCode(const QString& code);

[[nodiscard]] ThemePreference theme(); // default System
void setTheme(ThemePreference theme);

} // namespace AppConfig
