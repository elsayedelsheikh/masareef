#include "models/expensemodel.h"

#include "utils/currencyformatter.h"

#include <QHash>
#include <QSqlError>
#include <QSqlQuery>

namespace {
bool reportError(const QSqlQuery& query, QString* error)
{
    if (error)
        *error = query.lastError().text();
    return false;
}
} // namespace

ExpenseModel::ExpenseModel(QObject* parent)
    : QSqlQueryModel(parent)
{
}

void ExpenseModel::setDateRange(const QDate& from, const QDate& to)
{
    m_from = from;
    m_to = to;
}

void ExpenseModel::setCategoryFilter(int categoryId)
{
    m_categoryId = categoryId;
}

void ExpenseModel::refresh()
{
    QString sql = QStringLiteral(
        "SELECT e.id, e.expense_date, c.name, e.description, e.amount, e.notes "
        "FROM expenses e JOIN categories c ON c.id = e.category_id WHERE 1=1");
    if (m_from.isValid() && m_to.isValid())
        sql += QStringLiteral(" AND e.expense_date BETWEEN :from AND :to");
    if (m_categoryId > 0)
        sql += QStringLiteral(" AND e.category_id = :cat");
    sql += QStringLiteral(" ORDER BY e.expense_date DESC, e.id DESC");

    QSqlQuery query;
    query.prepare(sql);
    if (m_from.isValid() && m_to.isValid()) {
        query.bindValue(QStringLiteral(":from"), m_from.toString(Qt::ISODate));
        query.bindValue(QStringLiteral(":to"), m_to.toString(Qt::ISODate));
    }
    if (m_categoryId > 0)
        query.bindValue(QStringLiteral(":cat"), m_categoryId);
    query.exec();
    setQuery(std::move(query));

    setHeaderData(ColDate, Qt::Horizontal, tr("Date"));
    setHeaderData(ColCategory, Qt::Horizontal, tr("Category"));
    setHeaderData(ColDescription, Qt::Horizontal, tr("Description"));
    setHeaderData(ColAmount, Qt::Horizontal, tr("Amount"));
    setHeaderData(ColNotes, Qt::Horizontal, tr("Notes"));

    m_total = totalBetween(m_from, m_to, m_categoryId);
}

int ExpenseModel::expenseIdAt(int row) const
{
    return QSqlQueryModel::data(index(row, ColId)).toInt();
}

QVariant ExpenseModel::data(const QModelIndex& index, int role) const
{
    if (index.column() == ColAmount) {
        if (role == Qt::DisplayRole)
            return CurrencyFormatter::format(QSqlQueryModel::data(index).toLongLong());
        if (role == Qt::TextAlignmentRole)
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QSqlQueryModel::data(index, role);
}

bool ExpenseModel::addExpense(int categoryId, qint64 amount, const QString& description,
                              const QDate& date, const QString& notes,
                              const QVariant& recurringBillId, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO expenses (category_id, amount, description, expense_date, notes, "
        "recurring_bill_id) VALUES (?, ?, ?, ?, ?, ?)"));
    query.addBindValue(categoryId);
    query.addBindValue(amount);
    query.addBindValue(description);
    query.addBindValue(date.toString(Qt::ISODate));
    query.addBindValue(notes);
    query.addBindValue(recurringBillId);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool ExpenseModel::updateExpense(int id, int categoryId, qint64 amount,
                                 const QString& description, const QDate& date,
                                 const QString& notes, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE expenses SET category_id = ?, amount = ?, description = ?, "
        "expense_date = ?, notes = ? WHERE id = ?"));
    query.addBindValue(categoryId);
    query.addBindValue(amount);
    query.addBindValue(description);
    query.addBindValue(date.toString(Qt::ISODate));
    query.addBindValue(notes);
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool ExpenseModel::deleteExpense(int id, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM expenses WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool ExpenseModel::fetchExpense(int id, Expense* out, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT id, category_id, amount, description, expense_date, notes "
        "FROM expenses WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    if (!query.next()) {
        if (error)
            *error = tr("Expense %1 not found").arg(id);
        return false;
    }
    if (out) {
        out->id = query.value(0).toInt();
        out->categoryId = query.value(1).toInt();
        out->amount = query.value(2).toLongLong();
        out->description = query.value(3).toString();
        out->date = QDate::fromString(query.value(4).toString(), Qt::ISODate);
        out->notes = query.value(5).toString();
    }
    return true;
}

qint64 ExpenseModel::totalBetween(const QDate& from, const QDate& to, int categoryId)
{
    QString sql = QStringLiteral("SELECT COALESCE(SUM(amount), 0) FROM expenses WHERE 1=1");
    if (from.isValid() && to.isValid())
        sql += QStringLiteral(" AND expense_date BETWEEN :from AND :to");
    if (categoryId > 0)
        sql += QStringLiteral(" AND category_id = :cat");

    QSqlQuery query;
    query.prepare(sql);
    if (from.isValid() && to.isValid()) {
        query.bindValue(QStringLiteral(":from"), from.toString(Qt::ISODate));
        query.bindValue(QStringLiteral(":to"), to.toString(Qt::ISODate));
    }
    if (categoryId > 0)
        query.bindValue(QStringLiteral(":cat"), categoryId);
    if (!query.exec() || !query.next())
        return 0;
    return query.value(0).toLongLong();
}

QList<ExpenseModel::CategoryTotal> ExpenseModel::totalsByCategory(const QDate& from,
                                                                  const QDate& to)
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
        t.total = query.value(3).toLongLong();
        totals.append(t);
    }
    return totals;
}

QList<ExpenseModel::MonthTotal> ExpenseModel::monthlyTotals(int months)
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
        entry.total = byMonth.value(entry.month.toString(QStringLiteral("yyyy-MM")), 0);
        result.append(entry);
    }
    return result;
}
