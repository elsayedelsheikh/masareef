#include "testutils.h"

#include "models/expensemodel.h"
#include "models/recurringbillmodel.h"

#include <QtTest>

class TestRecurringBillModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void advanceDueDate_data();
    void advanceDueDate();
    void addAndFetch_roundTrip();
    void updateBill_changesFields();
    void bills_activeOnlyFilter();
    void setActive_toggles();
    void markPaid_recordsExpenseAndAdvances();
    void markPaid_failsForUnknownBill();
    void removeBill_detachesExpenses();

private:
    int m_billsId = -1;

    RecurringBill makeBill(const QString& name, const QDate& due,
                           const QString& recurrence = QStringLiteral("monthly")) const;
};

void TestRecurringBillModel::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestRecurringBillModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    QVERIFY(m_billsId > 0);
}

RecurringBill TestRecurringBillModel::makeBill(const QString& name, const QDate& due,
                                               const QString& recurrence) const
{
    RecurringBill bill;
    bill.categoryId = m_billsId;
    bill.name = name;
    bill.amount = 35000;
    bill.recurrence = recurrence;
    bill.nextDue = due;
    bill.active = true;
    bill.notes = QStringLiteral("note");
    return bill;
}

void TestRecurringBillModel::advanceDueDate_data()
{
    QTest::addColumn<QDate>("from");
    QTest::addColumn<QString>("recurrence");
    QTest::addColumn<QDate>("expected");
    QTest::newRow("monthly plain")
        << QDate(2026, 1, 15) << QStringLiteral("monthly") << QDate(2026, 2, 15);
    QTest::newRow("monthly clamps Jan 31 -> Feb 28")
        << QDate(2026, 1, 31) << QStringLiteral("monthly") << QDate(2026, 2, 28);
    QTest::newRow("quarterly clamps Jan 31 -> Apr 30")
        << QDate(2026, 1, 31) << QStringLiteral("quarterly") << QDate(2026, 4, 30);
    QTest::newRow("yearly leap day -> Feb 28")
        << QDate(2024, 2, 29) << QStringLiteral("yearly") << QDate(2025, 2, 28);
}

void TestRecurringBillModel::advanceDueDate()
{
    QFETCH(QDate, from);
    QFETCH(QString, recurrence);
    QFETCH(QDate, expected);
    QCOMPARE(RecurringBillModel::advanceDueDate(from, recurrence), expected);
}

void TestRecurringBillModel::addAndFetch_roundTrip()
{
    QString error;
    int newId = 0;
    QVERIFY2(RecurringBillModel::addBill(
                 makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)), &error, &newId),
             qPrintable(error));
    QVERIFY(newId > 0);

    RecurringBill fetched;
    QVERIFY(RecurringBillModel::fetchBill(newId, &fetched, &error));
    QCOMPARE(fetched.name, QStringLiteral("Internet"));
    QCOMPARE(fetched.categoryId, m_billsId);
    QCOMPARE(fetched.categoryName, QStringLiteral("Bills"));
    QCOMPARE(fetched.amount, qint64(35000));
    QCOMPARE(fetched.recurrence, QStringLiteral("monthly"));
    QCOMPARE(fetched.nextDue, QDate(2026, 7, 10));
    QVERIFY(fetched.active);
    QCOMPARE(fetched.notes, QStringLiteral("note"));
}

void TestRecurringBillModel::updateBill_changesFields()
{
    QString error;
    int id = 0;
    QVERIFY(RecurringBillModel::addBill(
        makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)), &error, &id));

    RecurringBill changed;
    QVERIFY(RecurringBillModel::fetchBill(id, &changed, &error));
    changed.name = QStringLiteral("Fiber");
    changed.amount = 42000;
    changed.recurrence = QStringLiteral("quarterly");
    changed.nextDue = QDate(2026, 8, 1);
    QVERIFY2(RecurringBillModel::updateBill(changed, &error), qPrintable(error));

    RecurringBill fetched;
    QVERIFY(RecurringBillModel::fetchBill(id, &fetched, &error));
    QCOMPARE(fetched.name, QStringLiteral("Fiber"));
    QCOMPARE(fetched.amount, qint64(42000));
    QCOMPARE(fetched.recurrence, QStringLiteral("quarterly"));
    QCOMPARE(fetched.nextDue, QDate(2026, 8, 1));
}

void TestRecurringBillModel::bills_activeOnlyFilter()
{
    QString error;
    int activeId = 0, inactiveId = 0;
    QVERIFY(RecurringBillModel::addBill(
        makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)), &error, &activeId));
    QVERIFY(RecurringBillModel::addBill(
        makeBill(QStringLiteral("Old Gym"), QDate(2026, 7, 20)), &error, &inactiveId));
    QVERIFY(RecurringBillModel::setActive(inactiveId, false, &error));

    QCOMPARE(RecurringBillModel::bills(true).size(), 1);
    QCOMPARE(RecurringBillModel::bills(false).size(), 2);
    QCOMPARE(RecurringBillModel::bills(true).first().name, QStringLiteral("Internet"));
}

void TestRecurringBillModel::setActive_toggles()
{
    QString error;
    int id = 0;
    QVERIFY(RecurringBillModel::addBill(
        makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)), &error, &id));
    QVERIFY(RecurringBillModel::setActive(id, false, &error));
    RecurringBill fetched;
    QVERIFY(RecurringBillModel::fetchBill(id, &fetched, &error));
    QVERIFY(!fetched.active);
    QVERIFY(RecurringBillModel::setActive(id, true, &error));
    QVERIFY(RecurringBillModel::fetchBill(id, &fetched, &error));
    QVERIFY(fetched.active);
}

void TestRecurringBillModel::markPaid_recordsExpenseAndAdvances()
{
    QString error;
    int id = 0;
    QVERIFY(RecurringBillModel::addBill(
        makeBill(QStringLiteral("Electricity"), QDate(2026, 1, 31)), &error, &id));

    QVERIFY2(RecurringBillModel::markPaid(id, 37550, QDate(2026, 2, 2),
                                          QStringLiteral("Electricity"),
                                          QStringLiteral("winter rate"), &error),
             qPrintable(error));

    // The payment is a normal expense linked back to its template
    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral(
        "SELECT amount, description, expense_date, notes, recurring_bill_id, category_id "
        "FROM expenses")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toLongLong(), qint64(37550));
    QCOMPARE(query.value(1).toString(), QStringLiteral("Electricity"));
    QCOMPARE(QDate::fromString(query.value(2).toString(), Qt::ISODate), QDate(2026, 2, 2));
    QCOMPARE(query.value(3).toString(), QStringLiteral("winter rate"));
    QCOMPARE(query.value(4).toInt(), id);
    QCOMPARE(query.value(5).toInt(), m_billsId);
    QVERIFY(!query.next());

    // ...and the due date advanced one cycle with day-of-month clamping
    RecurringBill fetched;
    QVERIFY(RecurringBillModel::fetchBill(id, &fetched, &error));
    QCOMPARE(fetched.nextDue, QDate(2026, 2, 28));
}

void TestRecurringBillModel::markPaid_failsForUnknownBill()
{
    QString error;
    QVERIFY(!RecurringBillModel::markPaid(999999, 100, QDate(2026, 2, 2), QString(),
                                          QString(), &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestRecurringBillModel::removeBill_detachesExpenses()
{
    QString error;
    int id = 0;
    QVERIFY(RecurringBillModel::addBill(
        makeBill(QStringLiteral("Electricity"), QDate(2026, 7, 1)), &error, &id));
    QVERIFY(RecurringBillModel::markPaid(id, 100, QDate(2026, 7, 1), QStringLiteral("Electricity"),
                                         QString(), &error));

    QVERIFY2(RecurringBillModel::removeBill(id, &error), qPrintable(error));
    QCOMPARE(RecurringBillModel::bills(false).size(), 0);

    // ON DELETE SET NULL keeps the payment history
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);
    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("SELECT recurring_bill_id FROM expenses")));
    QVERIFY(query.next());
    QVERIFY(query.value(0).isNull());
}

QTEST_GUILESS_MAIN(TestRecurringBillModel)
#include "tst_recurringbillmodel.moc"
