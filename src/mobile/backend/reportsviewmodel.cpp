#include "backend/reportsviewmodel.h"

#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"
#include "utils/localeformat.h"
#include "utils/palette.h"

ReportsViewModel::ReportsViewModel(QObject* parent)
    : QObject(parent)
{
}

QList<MonthTotal> ReportsViewModel::monthlyTotals() const
{
    const auto totals = ExpenseRepository::monthlyTotals(12);
    QList<MonthTotal> result;

    for (const auto& total : totals) {
        MonthTotal m;
        m.month = total.month;
        // Through LocaleFormat, not QDate::toString: the latter picks the
        // system locale rather than the language the UI is actually running
        // in, so an Arabic UI on an English phone kept English month names.
        m.monthName = LocaleFormat::monthShortYear(total.month);
        m.monthShort = LocaleFormat::monthShort(total.month);
        m.totalFormatted = CurrencyFormatter::format(total.total);
        m.totalMinor = total.total.minorUnits();
        result.append(m);
    }

    return result;
}

QList<CategoryTotal> ReportsViewModel::categoryTotals(QDate from, QDate to) const
{
    const auto totals = ExpenseRepository::totalsByCategory(from, to);
    QList<CategoryTotal> result;

    for (const auto& total : totals) {
        CategoryTotal c;
        c.categoryId = total.categoryId;
        c.categoryName = total.name;
        c.categoryColor = Palette::series(total.color);
        c.totalFormatted = CurrencyFormatter::format(total.total);
        c.totalMinor = total.total.minorUnits();
        result.append(c);
    }

    return result;
}
