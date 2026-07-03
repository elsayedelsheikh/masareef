#pragma once

#include "core/money.h"
#include "core/recurrence.h"
#include "core/result.h"

#include <QDate>
#include <QList>
#include <QString>

struct RecurringBill {
    int id = 0;
    int categoryId = 0;
    QString categoryName;
    QString name;
    Money amount; // expected amount, editable at payment time
    Recurrence recurrence = Recurrence::Monthly;
    QDate nextDue;
    bool active = true;
    QString notes;

    [[nodiscard]] bool isOverdue() const { return nextDue < QDate::currentDate(); }
    [[nodiscard]] int daysUntilDue() const
    {
        return int(QDate::currentDate().daysTo(nextDue));
    }
};

// Storage access for recurring bill templates and the Mark Paid
// transaction (insert the payment expense + advance next_due_date
// atomically).
namespace BillRepository {

[[nodiscard]] QList<RecurringBill> all(bool activeOnly);
[[nodiscard]] Result<RecurringBill> fetch(int id);
[[nodiscard]] Result<int> add(const RecurringBill& bill); // returns the new id
[[nodiscard]] Result<void> update(const RecurringBill& bill);
[[nodiscard]] Result<void> remove(int id);
[[nodiscard]] Result<void> setActive(int id, bool active);
[[nodiscard]] Result<void> markPaid(int id, Money amount, QDate paidDate,
                                    const QString& description, const QString& notes);

} // namespace BillRepository
