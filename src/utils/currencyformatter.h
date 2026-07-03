#pragma once

#include "core/money.h"

#include <QString>

#include <optional>

// Rendering and parsing of Money for the UI. Amounts always display with
// the ISO currency code (e.g. "1,234.50 EGP") — never a localized symbol.
namespace CurrencyFormatter {

// 10050 minor units -> "100.50 EGP"
[[nodiscard]] QString format(Money amount);
// Amount without the currency code: 10050 -> "100.50"
[[nodiscard]] QString formatPlain(Money amount);
// "100.5" / "100,5" -> 10050 minor units. Rejects negatives and garbage.
[[nodiscard]] std::optional<Money> parse(const QString& text);

} // namespace CurrencyFormatter
