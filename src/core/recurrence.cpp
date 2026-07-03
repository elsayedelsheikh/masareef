#include "core/recurrence.h"

#include <QCoreApplication>

QString toDbString(Recurrence recurrence)
{
    switch (recurrence) {
    case Recurrence::Monthly:
        return QStringLiteral("monthly");
    case Recurrence::Quarterly:
        return QStringLiteral("quarterly");
    case Recurrence::Yearly:
        return QStringLiteral("yearly");
    }
    return QStringLiteral("monthly");
}

std::optional<Recurrence> recurrenceFromDbString(const QString& value)
{
    if (value == QLatin1String("monthly"))
        return Recurrence::Monthly;
    if (value == QLatin1String("quarterly"))
        return Recurrence::Quarterly;
    if (value == QLatin1String("yearly"))
        return Recurrence::Yearly;
    return std::nullopt;
}

QString displayLabel(Recurrence recurrence)
{
    switch (recurrence) {
    case Recurrence::Quarterly:
        return QCoreApplication::translate("Recurrence", "Quarterly");
    case Recurrence::Yearly:
        return QCoreApplication::translate("Recurrence", "Yearly");
    case Recurrence::Monthly:
        break;
    }
    return QCoreApplication::translate("Recurrence", "Monthly");
}

QDate advance(QDate from, Recurrence recurrence)
{
    switch (recurrence) {
    case Recurrence::Quarterly:
        return from.addMonths(3);
    case Recurrence::Yearly:
        return from.addYears(1);
    case Recurrence::Monthly:
        break;
    }
    return from.addMonths(1);
}
