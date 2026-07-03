#include "testutils.h"

#include "storage/expenserepository.h"

#include <QtTest>

class TestExpenseRepository : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void addAndFetch_roundTrip();
    void fetch_unknownIdFails();
    void update_changesFields();
    void remove_deletesRow();
    void removeMany_deletesAll();
    void totalFor_respectsFilter();
    void totalsByCategory_sortedDescending();
    void monthlyTotals_zeroFillsGaps();

private:
    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestExpenseRepository::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestExpenseRepository::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

void TestExpenseRepository::addAndFetch_roundTrip()
{
    const Result<int> added =
        ExpenseRepository::add({ .categoryId = m_billsId,
                                 .amount = Money::fromMinorUnits(25099),
                                 .description = QStringLiteral("Electricity"),
                                 .date = QDate(2026, 6, 15),
                                 .notes = QStringLiteral("June meter") });
    VERIFY_OK(added);
    QVERIFY(*added > 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);

    const Result<Expense> fetched = ExpenseRepository::fetch(*added);
    VERIFY_OK(fetched);
    QCOMPARE(fetched->categoryId, m_billsId);
    QCOMPARE(fetched->amount, Money::fromMinorUnits(25099));
    QCOMPARE(fetched->description, QStringLiteral("Electricity"));
    QCOMPARE(fetched->date, QDate(2026, 6, 15));
    QCOMPARE(fetched->notes, QStringLiteral("June meter"));
    QVERIFY(!fetched->recurringBillId.has_value());
}

void TestExpenseRepository::fetch_unknownIdFails()
{
    const Result<Expense> missing = ExpenseRepository::fetch(999999);
    QVERIFY(!missing);
    QVERIFY(!missing.error().message.isEmpty());
}

void TestExpenseRepository::update_changesFields()
{
    const Result<int> added =
        ExpenseRepository::add({ .categoryId = m_billsId,
                                 .amount = Money::fromMinorUnits(1000),
                                 .description = QStringLiteral("Water"),
                                 .date = QDate(2026, 6, 1) });
    VERIFY_OK(added);

    VERIFY_OK(ExpenseRepository::update({ .id = *added,
                                          .categoryId = m_groceriesId,
                                          .amount = Money::fromMinorUnits(2000),
                                          .description = QStringLiteral("Market"),
                                          .date = QDate(2026, 6, 2),
                                          .notes = QStringLiteral("updated") }));

    const Result<Expense> fetched = ExpenseRepository::fetch(*added);
    VERIFY_OK(fetched);
    QCOMPARE(fetched->categoryId, m_groceriesId);
    QCOMPARE(fetched->amount, Money::fromMinorUnits(2000));
    QCOMPARE(fetched->description, QStringLiteral("Market"));
    QCOMPARE(fetched->date, QDate(2026, 6, 2));
    QCOMPARE(fetched->notes, QStringLiteral("updated"));
}

void TestExpenseRepository::remove_deletesRow()
{
    const Result<int> added = ExpenseRepository::add(
        { .categoryId = m_billsId, .amount = Money::fromMinorUnits(1000),
          .date = QDate(2026, 6, 1) });
    VERIFY_OK(added);
    VERIFY_OK(ExpenseRepository::remove(*added));
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseRepository::removeMany_deletesAll()
{
    QList<int> ids;
    for (int day = 1; day <= 3; ++day) {
        const Result<int> added = ExpenseRepository::add(
            { .categoryId = m_billsId, .amount = Money::fromMinorUnits(100),
              .date = QDate(2026, 6, day) });
        VERIFY_OK(added);
        ids.append(*added);
    }
    VERIFY_OK(ExpenseRepository::removeMany(ids));
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseRepository::totalFor_respectsFilter()
{
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(100),
                                       .description = QStringLiteral("May bill"),
                                       .date = QDate(2026, 5, 20) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(200),
                                       .description = QStringLiteral("June bill"),
                                       .date = QDate(2026, 6, 1) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_groceriesId,
                                       .amount = Money::fromMinorUnits(300),
                                       .description = QStringLiteral("June market"),
                                       .date = QDate(2026, 6, 30) }));

    const ExpenseFilter june { .from = QDate(2026, 6, 1), .to = QDate(2026, 6, 30) };
    QCOMPARE(ExpenseRepository::totalFor(june), Money::fromMinorUnits(500));

    ExpenseFilter juneBills = june;
    juneBills.categoryId = m_billsId;
    QCOMPARE(ExpenseRepository::totalFor(juneBills), Money::fromMinorUnits(200));

    ExpenseFilter juneMarket = june;
    juneMarket.searchText = QStringLiteral("market");
    QCOMPARE(ExpenseRepository::totalFor(juneMarket), Money::fromMinorUnits(300));
}

void TestExpenseRepository::totalsByCategory_sortedDescending()
{
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(100),
                                       .date = QDate(2026, 6, 1) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(150),
                                       .date = QDate(2026, 6, 5) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_groceriesId,
                                       .amount = Money::fromMinorUnits(300),
                                       .date = QDate(2026, 6, 9) }));
    // Outside the queried range, must not show up
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_groceriesId,
                                       .amount = Money::fromMinorUnits(999),
                                       .date = QDate(2026, 7, 1) }));

    const QList<ExpenseRepository::CategoryTotal> totals =
        ExpenseRepository::totalsByCategory(QDate(2026, 6, 1), QDate(2026, 6, 30));
    QCOMPARE(totals.size(), 2);
    QCOMPARE(totals.at(0).name, QStringLiteral("Groceries"));
    QCOMPARE(totals.at(0).total, Money::fromMinorUnits(300));
    QCOMPARE(totals.at(1).name, QStringLiteral("Bills"));
    QCOMPARE(totals.at(1).total, Money::fromMinorUnits(250));
    QVERIFY(!totals.at(0).color.isEmpty());
}

void TestExpenseRepository::monthlyTotals_zeroFillsGaps()
{
    const QDate now = QDate::currentDate();
    const QDate thisMonth(now.year(), now.month(), 1);
    const QDate threeBack = thisMonth.addMonths(-3);
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(400),
                                       .date = thisMonth.addDays(5) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(700),
                                       .date = threeBack.addDays(2) }));

    const QList<ExpenseRepository::MonthTotal> months =
        ExpenseRepository::monthlyTotals(12);
    QCOMPARE(months.size(), 12);
    QCOMPARE(months.last().month, thisMonth);
    QCOMPARE(months.last().total, Money::fromMinorUnits(400));
    QCOMPARE(months.at(8).month, threeBack);
    QCOMPARE(months.at(8).total, Money::fromMinorUnits(700));
    QCOMPARE(months.at(7).total, Money{});
}

QTEST_GUILESS_MAIN(TestExpenseRepository)
#include "tst_expenserepository.moc"
