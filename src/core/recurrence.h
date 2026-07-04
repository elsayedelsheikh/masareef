#pragma once

#include <QDate>
#include <QObject>
#include <QString>

#include <optional>

// How often a recurring bill comes due. The database stores the lowercase
// string form; everything above the storage layer works with the enum so
// a typo'd string cannot travel through the program.
namespace RecurrenceNS {
Q_NAMESPACE

enum class Recurrence { Monthly, Quarterly, Yearly };
Q_ENUM_NS(Recurrence)

} // namespace RecurrenceNS

using Recurrence = RecurrenceNS::Recurrence;

[[nodiscard]] QString toDbString(Recurrence recurrence);
[[nodiscard]] std::optional<Recurrence> recurrenceFromDbString(const QString& value);

// Translated, capitalized label for UI display.
[[nodiscard]] QString displayLabel(Recurrence recurrence);

// Next due date after `from`. QDate clamps the day-of-month automatically
// (Jan 31 -> Feb 28).
[[nodiscard]] QDate advance(QDate from, Recurrence recurrence);
