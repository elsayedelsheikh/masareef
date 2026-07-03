#include "testutils.h"

#include "storage/billrepository.h"

#include <QtTest>

class TestBillRepository : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void addAndFetch_roundTrip();
    void update_changesFields();
    void all_activeOnlyFilter();
    void setActive_toggles();
    void markPaid_recordsExpenseAndAdvances();
    void markPaid_failsForUnknownBill();
    void remove_detachesExpenses();

private:
    int m_billsId = -1;

    [[nodiscard]] RecurringBill makeBill(const QString& name, QDate due,
                                         Recurrence recurrence
                                         = Recurrence::Monthly) const;
};

void TestBillRepository::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestBillRepository::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    QVERIFY(m_billsId > 0);
}

RecurringBill TestBillRepository::makeBill(const QString& name, QDate due,
                                           Recurrence recurrence) const
{
    return RecurringBill { .categoryId = m_billsId,
                           .name = name,
                           .amount = Money::fromMinorUnits(35000),
                           .recurrence = recurrence,
                           .nextDue = due,
                           .active = true,
                           .notes = QStringLiteral("note") };
}

void TestBillRepository::addAndFetch_roundTrip()
{
    const Result<int> added =
        BillRepository::add(makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)));
    VERIFY_OK(added);
    QVERIFY(*added > 0);

    const Result<RecurringBill> fetched = BillRepository::fetch(*added);
    VERIFY_OK(fetched);
    QCOMPARE(fetched->name, QStringLiteral("Internet"));
    QCOMPARE(fetched->categoryId, m_billsId);
    QCOMPARE(fetched->categoryName, QStringLiteral("Bills"));
    QCOMPARE(fetched->amount, Money::fromMinorUnits(35000));
    QCOMPARE(fetched->recurrence, Recurrence::Monthly);
    QCOMPARE(fetched->nextDue, QDate(2026, 7, 10));
    QVERIFY(fetched->active);
    QCOMPARE(fetched->notes, QStringLiteral("note"));
}

void TestBillRepository::update_changesFields()
{
    const Result<int> added =
        BillRepository::add(makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)));
    VERIFY_OK(added);

    Result<RecurringBill> changed = BillRepository::fetch(*added);
    VERIFY_OK(changed);
    changed->name = QStringLiteral("Fiber");
    changed->amount = Money::fromMinorUnits(42000);
    changed->recurrence = Recurrence::Quarterly;
    changed->nextDue = QDate(2026, 8, 1);
    VERIFY_OK(BillRepository::update(*changed));

    const Result<RecurringBill> fetched = BillRepository::fetch(*added);
    VERIFY_OK(fetched);
    QCOMPARE(fetched->name, QStringLiteral("Fiber"));
    QCOMPARE(fetched->amount, Money::fromMinorUnits(42000));
    QCOMPARE(fetched->recurrence, Recurrence::Quarterly);
    QCOMPARE(fetched->nextDue, QDate(2026, 8, 1));
}

void TestBillRepository::all_activeOnlyFilter()
{
    VERIFY_OK(BillRepository::add(makeBill(QStringLiteral("Internet"),
                                           QDate(2026, 7, 10))));
    const Result<int> inactive =
        BillRepository::add(makeBill(QStringLiteral("Old Gym"), QDate(2026, 7, 20)));
    VERIFY_OK(inactive);
    VERIFY_OK(BillRepository::setActive(*inactive, false));

    QCOMPARE(BillRepository::all(true).size(), 1);
    QCOMPARE(BillRepository::all(false).size(), 2);
    QCOMPARE(BillRepository::all(true).first().name, QStringLiteral("Internet"));
}

void TestBillRepository::setActive_toggles()
{
    const Result<int> added =
        BillRepository::add(makeBill(QStringLiteral("Internet"), QDate(2026, 7, 10)));
    VERIFY_OK(added);

    VERIFY_OK(BillRepository::setActive(*added, false));
    Result<RecurringBill> fetched = BillRepository::fetch(*added);
    VERIFY_OK(fetched);
    QVERIFY(!fetched->active);

    VERIFY_OK(BillRepository::setActive(*added, true));
    fetched = BillRepository::fetch(*added);
    VERIFY_OK(fetched);
    QVERIFY(fetched->active);
}

void TestBillRepository::markPaid_recordsExpenseAndAdvances()
{
    const Result<int> added =
        BillRepository::add(makeBill(QStringLiteral("Electricity"), QDate(2026, 1, 31)));
    VERIFY_OK(added);

    VERIFY_OK(BillRepository::markPaid(*added, Money::fromMinorUnits(37550),
                                       QDate(2026, 2, 2), QStringLiteral("Electricity"),
                                       QStringLiteral("winter rate")));

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
    QCOMPARE(query.value(4).toInt(), *added);
    QCOMPARE(query.value(5).toInt(), m_billsId);
    QVERIFY(!query.next());

    // ...and the due date advanced one cycle with day-of-month clamping
    const Result<RecurringBill> fetched = BillRepository::fetch(*added);
    VERIFY_OK(fetched);
    QCOMPARE(fetched->nextDue, QDate(2026, 2, 28));
}

void TestBillRepository::markPaid_failsForUnknownBill()
{
    const Result<void> paid = BillRepository::markPaid(
        999999, Money::fromMinorUnits(100), QDate(2026, 2, 2), QString(), QString());
    QVERIFY(!paid);
    QVERIFY(!paid.error().message.isEmpty());
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestBillRepository::remove_detachesExpenses()
{
    const Result<int> added =
        BillRepository::add(makeBill(QStringLiteral("Electricity"), QDate(2026, 7, 1)));
    VERIFY_OK(added);
    VERIFY_OK(BillRepository::markPaid(*added, Money::fromMinorUnits(100),
                                       QDate(2026, 7, 1), QStringLiteral("Electricity"),
                                       QString()));

    VERIFY_OK(BillRepository::remove(*added));
    QCOMPARE(BillRepository::all(false).size(), 0);

    // ON DELETE SET NULL keeps the payment history
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);
    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("SELECT recurring_bill_id FROM expenses")));
    QVERIFY(query.next());
    QVERIFY(query.value(0).isNull());
}

QTEST_GUILESS_MAIN(TestBillRepository)
#include "tst_billrepository.moc"
