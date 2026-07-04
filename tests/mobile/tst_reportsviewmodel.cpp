#include "testutils.h"

#include "backend/reportsviewmodel.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"

#include <QtTest>

class TestReportsViewModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void monthlyTotals_returnsLast12Months();
    void categoryTotals_returnsByCategoryInRange();

private:
    int addExpense(int categoryId, qint64 minor, QDate date);

    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestReportsViewModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestReportsViewModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

int TestReportsViewModel::addExpense(int categoryId, qint64 minor, QDate date)
{
    const Result<int> added = ExpenseRepository::add(
        { .categoryId = categoryId, .amount = Money::fromMinorUnits(minor),
          .description = QStringLiteral("test"), .date = date, .notes = {} });
    return added ? *added : -1;
}

void TestReportsViewModel::monthlyTotals_returnsLast12Months()
{
    const QDate today = QDate::currentDate();
    const QDate june2026(2026, 6, 15);
    const QDate july2026(2026, 7, 10);
    const QDate may2026(2026, 5, 20);

    addExpense(m_billsId, 1000, may2026);
    addExpense(m_groceriesId, 2000, june2026);
    addExpense(m_billsId, 1500, july2026);

    ReportsViewModel model;
    const auto months = model.monthlyTotals();

    // Should have entries for months with expenses
    QVERIFY(months.size() > 0);
    QVERIFY(months.size() <= 12);

    // Find June total (should be 2000 minor units = 20.00)
    bool foundJune = false;
    for (const auto& m : months) {
        if (m.month == QDate(2026, 6, 1)) {
            QCOMPARE(m.totalFormatted, CurrencyFormatter::format(Money::fromMinorUnits(2000)));
            foundJune = true;
            break;
        }
    }
    QVERIFY(foundJune);
}

void TestReportsViewModel::categoryTotals_returnsByCategoryInRange()
{
    const QDate june2026(2026, 6, 15);
    const QDate july2026(2026, 7, 10);

    addExpense(m_billsId, 1000, june2026);
    addExpense(m_groceriesId, 2000, june2026);
    addExpense(m_billsId, 1500, july2026); // Outside range

    ReportsViewModel model;
    const auto categories = model.categoryTotals(QDate(2026, 6, 1), QDate(2026, 6, 30));

    QCOMPARE(categories.size(), 2);

    // Categories sorted by total descending
    QCOMPARE(categories[0].categoryName, QStringLiteral("Groceries"));
    QCOMPARE(categories[0].totalFormatted,
             CurrencyFormatter::format(Money::fromMinorUnits(2000)));
    QCOMPARE(categories[1].categoryName, QStringLiteral("Bills"));
    QCOMPARE(categories[1].totalFormatted,
             CurrencyFormatter::format(Money::fromMinorUnits(1000)));
}

QTEST_GUILESS_MAIN(TestReportsViewModel)
#include "tst_reportsviewmodel.moc"
