#pragma once

#include "core/money.h"

#include <QDate>
#include <QLocale>
#include <QString>

// Locale-aware presentation of dates, shared by the QML layer (through
// AppBackend) and by the list models, so the same date rendered in a row
// and in a field always reads the same way.
//
// Digits are always Western (0-9), even in Arabic. Amounts are formatted
// that way by CurrencyFormatter, and a screen mixing "١٥ يناير" with
// "150.00 EGP" reads as broken rather than localized — so the month and
// weekday names localize while the numerals stay put.
namespace LocaleFormat {

// The locale the UI is actually running in: follows the stored language
// preference ("system" / "en" / "ar"), not the process locale.
[[nodiscard]] QLocale uiLocale();
// "ar_EG", "en_US", or the system locale name when the preference is
// "system" and the system is not Arabic.
[[nodiscard]] QString uiLocaleName();

[[nodiscard]] QString date(QDate date);      // 15 January 2026
[[nodiscard]] QString shortDate(QDate date); // 15/01/2026
[[nodiscard]] QString monthYear(QDate date);      // January 2026
[[nodiscard]] QString monthShort(QDate date);     // Jan
[[nodiscard]] QString monthShortYear(QDate date); // Jan 26

// "Today" / "Yesterday" / "Tomorrow", otherwise the long date. Used for
// the day headers in the expense list.
[[nodiscard]] QString relativeDate(QDate date);

// "Due today" / "Due tomorrow" / "Due in 5 days" / "Overdue by 3 days"
[[nodiscard]] QString dueLabel(QDate nextDue);

// Ungrouped "1234.50" — the form an amount field can round-trip through
// CurrencyFormatter::parse (formatPlain's thousands separators would be
// rejected on save).
[[nodiscard]] QString amountForEditing(Money amount);

} // namespace LocaleFormat
