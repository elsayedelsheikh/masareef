#include "backend/reportsviewmodel.h"

#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"
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
        m.monthName = total.month.toString(QStringLiteral("MMM yy"));
        m.monthShort = total.month.toString(QStringLiteral("MMM"));
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
