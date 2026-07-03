#include "storage/billrepository.h"

#include "storage/expenserepository.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

std::unexpected<Error> sqlFail(const QSqlQuery& query)
{
    return fail(query.lastError().text());
}

RecurringBill billFromQuery(const QSqlQuery& query)
{
    RecurringBill bill;
    bill.id = query.value(0).toInt();
    bill.categoryId = query.value(1).toInt();
    bill.name = query.value(2).toString();
    bill.amount = Money::fromMinorUnits(query.value(3).toLongLong());
    bill.recurrence =
        recurrenceFromDbString(query.value(4).toString()).value_or(Recurrence::Monthly);
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

namespace BillRepository {

QList<RecurringBill> all(bool activeOnly)
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

Result<RecurringBill> fetch(int id)
{
    QSqlQuery query;
    query.prepare(kSelectBill + QStringLiteral(" WHERE b.id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return sqlFail(query);
    if (!query.next())
        return fail(QCoreApplication::translate("BillRepository",
                                                "Recurring bill %1 not found").arg(id));
    return billFromQuery(query);
}

Result<int> add(const RecurringBill& bill)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO recurring_bills (category_id, name, amount, recurrence, "
        "next_due_date, is_active, notes) VALUES (?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(bill.categoryId);
    query.addBindValue(bill.name);
    query.addBindValue(qint64(bill.amount.minorUnits()));
    query.addBindValue(toDbString(bill.recurrence));
    query.addBindValue(bill.nextDue.toString(Qt::ISODate));
    query.addBindValue(bill.active ? 1 : 0);
    query.addBindValue(bill.notes);
    if (!query.exec())
        return sqlFail(query);
    return query.lastInsertId().toInt();
}

Result<void> update(const RecurringBill& bill)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "UPDATE recurring_bills SET category_id = ?, name = ?, amount = ?, "
        "recurrence = ?, next_due_date = ?, is_active = ?, notes = ? WHERE id = ?"));
    query.addBindValue(bill.categoryId);
    query.addBindValue(bill.name);
    query.addBindValue(qint64(bill.amount.minorUnits()));
    query.addBindValue(toDbString(bill.recurrence));
    query.addBindValue(bill.nextDue.toString(Qt::ISODate));
    query.addBindValue(bill.active ? 1 : 0);
    query.addBindValue(bill.notes);
    query.addBindValue(bill.id);
    if (!query.exec())
        return sqlFail(query);
    return {};
}

Result<void> remove(int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM recurring_bills WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return sqlFail(query);
    return {};
}

Result<void> setActive(int id, bool active)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE recurring_bills SET is_active = ? WHERE id = ?"));
    query.addBindValue(active ? 1 : 0);
    query.addBindValue(id);
    if (!query.exec())
        return sqlFail(query);
    return {};
}

Result<void> markPaid(int id, Money amount, QDate paidDate, const QString& description,
                      const QString& notes)
{
    const auto bill = fetch(id);
    if (!bill)
        return std::unexpected(bill.error());

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
        return fail(db.lastError().text());

    const auto step = [&]() -> Result<void> {
        Expense payment;
        payment.categoryId = bill->categoryId;
        payment.amount = amount;
        payment.description = description.isEmpty() ? bill->name : description;
        payment.date = paidDate;
        payment.notes = notes;
        payment.recurringBillId = id;
        if (auto added = ExpenseRepository::add(payment); !added)
            return std::unexpected(added.error());

        QSqlQuery query;
        query.prepare(QStringLiteral(
            "UPDATE recurring_bills SET next_due_date = ? WHERE id = ?"));
        query.addBindValue(
            advance(bill->nextDue, bill->recurrence).toString(Qt::ISODate));
        query.addBindValue(id);
        if (!query.exec())
            return sqlFail(query);
        return {};
    }();

    if (!step) {
        db.rollback();
        return step;
    }
    if (!db.commit()) {
        const QString message = db.lastError().text();
        db.rollback();
        return fail(message);
    }
    return {};
}

} // namespace BillRepository
