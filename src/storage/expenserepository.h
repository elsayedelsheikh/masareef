#pragma once

#include "core/money.h"
#include "core/result.h"

#include <QDate>
#include <QList>
#include <QSqlQuery>
#include <QString>

#include <optional>

struct Expense {
    int id = 0;
    int categoryId = 0;
    Money amount;
    QString description;
    QDate date;
    QString notes;
    std::optional<int> recurringBillId; // set when created via Mark Paid
};

// One value describes every way the expense list can be narrowed down;
// the list view, its total and the SQL stay in agreement by construction.
struct ExpenseFilter {
    QDate from;
    QDate to;
    int categoryId = -1; // -1 = all categories
    QString searchText;  // matches description, notes or category name
};

// Storage access for `expenses` plus the aggregations behind the
// dashboard and the charts.
namespace ExpenseRepository {

struct CategoryTotal {
    int categoryId = 0;
    QString name;
    QString color;
    Money total;
};

struct MonthTotal {
    QDate month; // first day of the month
    Money total;
};

[[nodiscard]] Result<int> add(const Expense& expense); // returns the new id
[[nodiscard]] Result<void> update(const Expense& expense);
[[nodiscard]] Result<void> remove(int id);
// All-or-nothing (one transaction); used by multi-select delete
[[nodiscard]] Result<void> removeMany(const QList<int>& ids);
[[nodiscard]] Result<Expense> fetch(int id);
[[nodiscard]] Result<QList<Expense>> fetchMany(const QList<int>& ids);

// Executed SELECT (id, date, category, description, amount, notes) for the
// expense table view, newest first.
[[nodiscard]] QSqlQuery makeFilteredQuery(const ExpenseFilter& filter);
[[nodiscard]] Money totalFor(const ExpenseFilter& filter);

// Per-category sums in the range, sorted by total descending
[[nodiscard]] QList<CategoryTotal> totalsByCategory(QDate from, QDate to);
// Last `months` calendar months including the current one, zero-filled
[[nodiscard]] QList<MonthTotal> monthlyTotals(int months);

} // namespace ExpenseRepository
