#include "storage/expenserepository.h"

#include <QCoreApplication>
#include <QHash>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

namespace {

std::unexpected<Error> sqlFail(const QSqlQuery& query)
{
    return fail(query.lastError().text());
}

// WHERE fragment + bind values for an ExpenseFilter. Expects the expenses
// table aliased `e` joined with categories aliased `c`.
struct WhereClause {
    QString sql; // starts with " AND ..." or empty
    QVariantList binds;
};

WhereClause whereFor(const ExpenseFilter& filter)
{
    WhereClause where;
    if (filter.from.isValid() && filter.to.isValid()) {
        where.sql += QStringLiteral(" AND e.expense_date BETWEEN ? AND ?");
        where.binds << filter.from.toString(Qt::ISODate) << filter.to.toString(Qt::ISODate);
    }
    if (filter.categoryId > 0) {
        where.sql += QStringLiteral(" AND e.category_id = ?");
        where.binds << filter.categoryId;
    }
    const QString needle = filter.searchText.trimmed();
    if (!needle.isEmpty()) {
        QString escaped = needle;
        escaped.replace(QLatin1Char('\\'), QLatin1String("\\\\"))
            .replace(QLatin1Char('%'), QLatin1String("\\%"))
            .replace(QLatin1Char('_'), QLatin1String("\\_"));
        const QString pattern = QLatin1Char('%') + escaped + QLatin1Char('%');
        where.sql += QStringLiteral(
            " AND (e.description LIKE ? ESCAPE '\\'"
            " OR e.notes LIKE ? ESCAPE '\\'"
            " OR c.name LIKE ? ESCAPE '\\')");
        where.binds << pattern << pattern << pattern;
    }
    return where;
}

Expense expenseFromQuery(const QSqlQuery& query)
{
    Expense expense;
    expense.id = query.value(0).toInt();
    expense.categoryId = query.value(1).toInt();
    expense.amount = Money::fromMinorUnits(query.value(2).toLongLong());
    expense.description = query.value(3).toString();
    expense.date = QDate::fromString(query.value(4).toString(), Qt::ISODate);
    expense.notes = query.value(5).toString();
    if (!query.value(6).isNull())
        expense.recurringBillId = query.value(6).toInt();
    return expense;
}

const QString kSelectExpense = QStringLiteral(
    "SELECT id, category_id, amount, description, expense_date, notes, "
    "recurring_bill_id FROM expenses");

} // namespace

namespace ExpenseRepository {

Result<int> add(const Expense& expense)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO expenses (category_id, amount, description, expense_date, notes, "
        "recurring_bill_id) VALUES (?, ?, ?, ?, ?, ?)"));
    query.addBindValue(expense.categoryId);
    query.addBindValue(qint64(expense.amount.minorUnits()));
    query.addBindValue(expense.description);
    query.addBindValue(expense.date.toString(Qt::ISODate));
    query.addBindValue(expense.notes);
    query.addBindValue(expense.recurringBillId ? QVariant(*expense.recurringBillId)
                                               : QVariant());
    if (!query.exec())
        return sqlFail(query);
    return query.lastInsertId().toInt();
}

Result<void> update(const Expense& expense)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE expenses SET category_id = ?, amount = ?, description = ?, "
        "expense_date = ?, notes = ? WHERE id = ?"));
    query.addBindValue(expense.categoryId);
    query.addBindValue(qint64(expense.amount.minorUnits()));
    query.addBindValue(expense.description);
    query.addBindValue(expense.date.toString(Qt::ISODate));
    query.addBindValue(expense.notes);
    query.addBindValue(expense.id);
    if (!query.exec())
        return sqlFail(query);
    return {};
}

Result<void> remove(int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM expenses WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return sqlFail(query);
    return {};
}

Result<void> removeMany(const QList<int>& ids)
{
    if (ids.isEmpty())
        return {};

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
        return fail(db.lastError().text());

    for (int id : ids) {
        if (auto res = remove(id); !res) {
            db.rollback();
            return res;
        }
    }
    if (!db.commit()) {
        const QString message = db.lastError().text();
        db.rollback();
        return fail(message);
    }
    return {};
}

Result<Expense> fetch(int id)
{
    QSqlQuery query;
    query.prepare(kSelectExpense + QStringLiteral(" WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return sqlFail(query);
    if (!query.next())
        return fail(QCoreApplication::translate("ExpenseRepository",
                                                "Expense %1 not found").arg(id));
    return expenseFromQuery(query);
}

Result<QList<Expense>> fetchMany(const QList<int>& ids)
{
    QList<Expense> expenses;
    expenses.reserve(ids.size());
    for (int id : ids) {
        auto expense = fetch(id);
        if (!expense)
            return std::unexpected(expense.error());
        expenses.append(std::move(*expense));
    }
    return expenses;
}

QSqlQuery makeFilteredQuery(const ExpenseFilter& filter)
{
    const WhereClause where = whereFor(filter);
    QSqlQuery query;
    query.prepare(QStringLiteral(
                      "SELECT e.id, e.expense_date, c.name, e.description, e.amount, "
                      "e.notes FROM expenses e JOIN categories c ON c.id = e.category_id "
                      "WHERE 1=1")
                  + where.sql + QStringLiteral(" ORDER BY e.expense_date DESC, e.id DESC"));
    for (const QVariant& value : where.binds)
        query.addBindValue(value);
    query.exec();
    return query;
}

Money totalFor(const ExpenseFilter& filter)
{
    const WhereClause where = whereFor(filter);
    QSqlQuery query;
    query.prepare(QStringLiteral(
                      "SELECT COALESCE(SUM(e.amount), 0) FROM expenses e "
                      "JOIN categories c ON c.id = e.category_id WHERE 1=1")
                  + where.sql);
    for (const QVariant& value : where.binds)
        query.addBindValue(value);
    if (!query.exec() || !query.next())
        return Money{};
    return Money::fromMinorUnits(query.value(0).toLongLong());
}

QList<CategoryTotal> totalsByCategory(QDate from, QDate to)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT c.id, c.name, c.color, SUM(e.amount) AS total "
        "FROM expenses e JOIN categories c ON c.id = e.category_id "
        "WHERE e.expense_date BETWEEN ? AND ? "
        "GROUP BY c.id ORDER BY total DESC"));
    query.addBindValue(from.toString(Qt::ISODate));
    query.addBindValue(to.toString(Qt::ISODate));

    QList<CategoryTotal> totals;
    if (!query.exec())
        return totals;
    while (query.next()) {
        CategoryTotal t;
        t.categoryId = query.value(0).toInt();
        t.name = query.value(1).toString();
        t.color = query.value(2).toString();
        t.total = Money::fromMinorUnits(query.value(3).toLongLong());
        totals.append(t);
    }
    return totals;
}

QList<MonthTotal> monthlyTotals(int months)
{
    const QDate today = QDate::currentDate();
    const QDate firstMonth = QDate(today.year(), today.month(), 1).addMonths(-(months - 1));

    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT strftime('%Y-%m', expense_date) AS month, SUM(amount) "
        "FROM expenses WHERE expense_date >= ? GROUP BY month"));
    query.addBindValue(firstMonth.toString(Qt::ISODate));

    QHash<QString, qint64> byMonth;
    if (query.exec()) {
        while (query.next())
            byMonth.insert(query.value(0).toString(), query.value(1).toLongLong());
    }

    QList<MonthTotal> result;
    result.reserve(months);
    for (int i = 0; i < months; ++i) {
        MonthTotal entry;
        entry.month = firstMonth.addMonths(i);
        entry.total = Money::fromMinorUnits(
            byMonth.value(entry.month.toString(QStringLiteral("yyyy-MM")), 0));
        result.append(entry);
    }
    return result;
}

} // namespace ExpenseRepository
