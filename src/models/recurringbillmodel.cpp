#include "models/recurringbillmodel.h"

#include "models/expensemodel.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
bool reportError(const QSqlQuery& query, QString* error)
{
    if (error)
        *error = query.lastError().text();
    return false;
}

RecurringBill billFromQuery(const QSqlQuery& query)
{
    RecurringBill bill;
    bill.id = query.value(0).toInt();
    bill.categoryId = query.value(1).toInt();
    bill.name = query.value(2).toString();
    bill.amount = query.value(3).toLongLong();
    bill.recurrence = query.value(4).toString();
    bill.nextDue = QDate::fromString(query.value(5).toString(), Qt::ISODate);
    bill.active = query.value(6).toBool();
    bill.notes = query.value(7).toString();
    bill.categoryName = query.value(8).toString();
    return bill;
}

const QString kSelectBill = QStringLiteral(
    "SELECT b.id, b.category_id, b.name, b.amount, b.recurrence, b.next_due_date, "
    "b.is_active, b.notes, c.name "
    "FROM recurring_bills b JOIN categories c ON c.id = b.category_id");
} // namespace

QList<RecurringBill> RecurringBillModel::bills(bool activeOnly)
{
    QList<RecurringBill> list;
    QSqlQuery query;
    QString sql = kSelectBill;
    if (activeOnly)
        sql += QStringLiteral(" WHERE b.is_active = 1");
    sql += QStringLiteral(" ORDER BY b.next_due_date, b.name");
    if (!query.exec(sql))
        return list;
    while (query.next())
        list.append(billFromQuery(query));
    return list;
}

bool RecurringBillModel::fetchBill(int id, RecurringBill* out, QString* error)
{
    QSqlQuery query;
    query.prepare(kSelectBill + QStringLiteral(" WHERE b.id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    if (!query.next()) {
        if (error)
            *error = QCoreApplication::translate("RecurringBillModel",
                                                 "Recurring bill %1 not found").arg(id);
        return false;
    }
    if (out)
        *out = billFromQuery(query);
    return true;
}

bool RecurringBillModel::addBill(const RecurringBill& bill, QString* error, int* newId)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO recurring_bills (category_id, name, amount, recurrence, "
        "next_due_date, is_active, notes) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(bill.categoryId);
    query.addBindValue(bill.name);
    query.addBindValue(bill.amount);
    query.addBindValue(bill.recurrence);
    query.addBindValue(bill.nextDue.toString(Qt::ISODate));
    query.addBindValue(bill.active ? 1 : 0);
    query.addBindValue(bill.notes);
    if (!query.exec())
        return reportError(query, error);
    if (newId)
        *newId = query.lastInsertId().toInt();
    return true;
}

bool RecurringBillModel::updateBill(const RecurringBill& bill, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE recurring_bills SET category_id = ?, name = ?, amount = ?, "
        "recurrence = ?, next_due_date = ?, is_active = ?, notes = ? WHERE id = ?"));
    query.addBindValue(bill.categoryId);
    query.addBindValue(bill.name);
    query.addBindValue(bill.amount);
    query.addBindValue(bill.recurrence);
    query.addBindValue(bill.nextDue.toString(Qt::ISODate));
    query.addBindValue(bill.active ? 1 : 0);
    query.addBindValue(bill.notes);
    query.addBindValue(bill.id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool RecurringBillModel::removeBill(int id, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM recurring_bills WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool RecurringBillModel::setActive(int id, bool active, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE recurring_bills SET is_active = ? WHERE id = ?"));
    query.addBindValue(active ? 1 : 0);
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool RecurringBillModel::markPaid(int id, qint64 amount, const QDate& paidDate,
                                  const QString& description, const QString& notes,
                                  QString* error)
{
    RecurringBill bill;
    if (!fetchBill(id, &bill, error))
        return false;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    const bool ok =
        ExpenseModel::addExpense(bill.categoryId, amount,
                                 description.isEmpty() ? bill.name : description,
                                 paidDate, notes, id, error)
        && [&] {
               QSqlQuery query;
               query.prepare(QStringLiteral(
                   "UPDATE recurring_bills SET next_due_date = ? WHERE id = ?"));
               query.addBindValue(
                   advanceDueDate(bill.nextDue, bill.recurrence).toString(Qt::ISODate));
               query.addBindValue(id);
               if (!query.exec())
                   return reportError(query, error);
               return true;
           }();

    if (!ok) {
        db.rollback();
        return false;
    }
    if (!db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}

QDate RecurringBillModel::advanceDueDate(const QDate& from, const QString& recurrence)
{
    if (recurrence == QLatin1String("quarterly"))
        return from.addMonths(3);
    if (recurrence == QLatin1String("yearly"))
        return from.addYears(1);
    return from.addMonths(1);
}

QString RecurringBillModel::recurrenceLabel(const QString& recurrence)
{
    if (recurrence == QLatin1String("quarterly"))
        return QCoreApplication::translate("RecurringBillModel", "Quarterly");
    if (recurrence == QLatin1String("yearly"))
        return QCoreApplication::translate("RecurringBillModel", "Yearly");
    return QCoreApplication::translate("RecurringBillModel", "Monthly");
}
