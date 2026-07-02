#pragma once

#include <QDate>
#include <QList>
#include <QString>

struct RecurringBill {
    int id = 0;
    int categoryId = 0;
    QString categoryName;
    QString name;
    qint64 amount = 0; // expected amount in minor units, editable at payment time
    QString recurrence; // 'monthly' | 'quarterly' | 'yearly'
    QDate nextDue;
    bool active = true;
    QString notes;

    bool isOverdue() const { return nextDue < QDate::currentDate(); }
    int daysUntilDue() const { return int(QDate::currentDate().daysTo(nextDue)); }
};

// CRUD for recurring bill templates and the Mark Paid transaction
// (insert the payment expense + advance next_due_date atomically).
class RecurringBillModel {
public:
    static QList<RecurringBill> bills(bool activeOnly);
    static bool fetchBill(int id, RecurringBill* out, QString* error = nullptr);
    static bool addBill(const RecurringBill& bill, QString* error = nullptr,
                        int* newId = nullptr);
    static bool updateBill(const RecurringBill& bill, QString* error = nullptr);
    static bool removeBill(int id, QString* error = nullptr);
    static bool setActive(int id, bool active, QString* error = nullptr);
    static bool markPaid(int id, qint64 amount, const QDate& paidDate,
                         const QString& description, const QString& notes,
                         QString* error = nullptr);

    // QDate clamps the day-of-month automatically (Jan 31 -> Feb 28)
    static QDate advanceDueDate(const QDate& from, const QString& recurrence);
    static QString recurrenceLabel(const QString& recurrence);
};
