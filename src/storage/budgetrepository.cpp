#include "storage/budgetrepository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

// Replace-or-clear one budget row. category NULL addresses the overall
// budget row (the unique COALESCE index keeps it single).
Result<void> setBudgetRow(const QVariant& categoryId, std::optional<Money> amount)
{
    QSqlQuery query;
    if (categoryId.isNull())
        query.prepare(QStringLiteral("DELETE FROM budgets WHERE category_id IS NULL"));
    else {
        query.prepare(QStringLiteral("DELETE FROM budgets WHERE category_id = ?"));
        query.addBindValue(categoryId);
    }
    if (!query.exec())
        return fail(query.lastError().text());

    if (!amount)
        return {};

    query.prepare(QStringLiteral("INSERT INTO budgets (category_id, amount) VALUES (?, ?)"));
    query.addBindValue(categoryId);
    query.addBindValue(qint64(amount->minorUnits()));
    if (!query.exec())
        return fail(query.lastError().text());
    return {};
}

} // namespace

namespace BudgetRepository {

std::optional<Money> overallBudget()
{
    QSqlQuery query;
    if (!query.exec(QStringLiteral(
            "SELECT amount FROM budgets WHERE category_id IS NULL"))
        || !query.next())
        return std::nullopt;
    return Money::fromMinorUnits(query.value(0).toLongLong());
}

QHash<int, Money> categoryBudgets()
{
    QHash<int, Money> budgets;
    QSqlQuery query;
    if (!query.exec(QStringLiteral(
            "SELECT category_id, amount FROM budgets WHERE category_id IS NOT NULL")))
        return budgets;
    while (query.next())
        budgets.insert(query.value(0).toInt(),
                       Money::fromMinorUnits(query.value(1).toLongLong()));
    return budgets;
}

Result<void> setOverallBudget(std::optional<Money> amount)
{
    return setBudgetRow(QVariant(), amount);
}

Result<void> setCategoryBudget(int categoryId, std::optional<Money> amount)
{
    return setBudgetRow(categoryId, amount);
}

} // namespace BudgetRepository
