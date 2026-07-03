#include "testutils.h"

#include "backend/dashboardviewmodel.h"
#include "backend/expensecontroller.h"
#include "storage/budgetrepository.h"
#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

#include <QAbstractItemModel>
#include <QSignalSpy>
#include <QtTest>

class TestDashboardViewModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void monthTotal_countsCurrentMonthOnly();
    void noBudget_reportsNoProgress();
    void budgetProgress_reflectsSpend();
    void overBudget_clampsProgressAndFlags();
    void topCategories_sortedWithShares();
    void topCategories_limitedToThree();
    void refresh_onControllerSignalsUpdatesTotals();

private:
    int addExpense(int categoryId, qint64 minor, QDate date);

    int m_billsId = -1;
    int m_groceriesId = -1;
    int m_otherId = -1;
    QDate m_today;
};

void TestDashboardViewModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestDashboardViewModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    Palette::setMode(Palette::Mode::Light);
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    m_otherId = TestUtils::categoryId(QStringLiteral("Other"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
    QVERIFY(m_otherId > 0);
    m_today = QDate::currentDate();
}

int TestDashboardViewModel::addExpense(int categoryId, qint64 minor, QDate date)
{
    const Result<int> added =
        ExpenseRepository::add({ .categoryId = categoryId,
                                 .amount = Money::fromMinorUnits(minor),
                                 .description = QStringLiteral("x"),
                                 .date = date });
    return added ? *added : -1;
}

void TestDashboardViewModel::monthTotal_countsCurrentMonthOnly()
{
    QVERIFY(addExpense(m_billsId, 10000, m_today) > 0);
    QVERIFY(addExpense(m_billsId, 5000, QDate(m_today.year(), m_today.month(), 1)) > 0);
    QVERIFY(addExpense(m_billsId, 77700, m_today.addMonths(-1)) > 0); // outside

    DashboardViewModel dashboard;
    QCOMPARE(dashboard.spentMinor(), qint64(15000));
    QCOMPARE(dashboard.monthTotalFormatted(),
             CurrencyFormatter::format(Money::fromMinorUnits(15000)));
}

void TestDashboardViewModel::noBudget_reportsNoProgress()
{
    QVERIFY(addExpense(m_billsId, 10000, m_today) > 0);

    DashboardViewModel dashboard;
    QCOMPARE(dashboard.hasBudget(), false);
    QCOMPARE(dashboard.budgetMinor(), qint64(0));
    QCOMPARE(dashboard.progress(), 0.0);
    QCOMPARE(dashboard.overBudget(), false);
}

void TestDashboardViewModel::budgetProgress_reflectsSpend()
{
    VERIFY_OK(BudgetRepository::setOverallBudget(Money::fromMinorUnits(40000)));
    QVERIFY(addExpense(m_billsId, 10000, m_today) > 0);

    DashboardViewModel dashboard;
    QCOMPARE(dashboard.hasBudget(), true);
    QCOMPARE(dashboard.budgetMinor(), qint64(40000));
    QCOMPARE(dashboard.progress(), 0.25);
    QCOMPARE(dashboard.overBudget(), false);
}

void TestDashboardViewModel::overBudget_clampsProgressAndFlags()
{
    VERIFY_OK(BudgetRepository::setOverallBudget(Money::fromMinorUnits(10000)));
    QVERIFY(addExpense(m_billsId, 15000, m_today) > 0);

    DashboardViewModel dashboard;
    QCOMPARE(dashboard.progress(), 1.0);
    QCOMPARE(dashboard.overBudget(), true);
}

void TestDashboardViewModel::topCategories_sortedWithShares()
{
    QVERIFY(addExpense(m_groceriesId, 30000, m_today) > 0);
    QVERIFY(addExpense(m_billsId, 10000, m_today) > 0);

    DashboardViewModel dashboard;
    QAbstractItemModel* top = dashboard.topCategories();
    QVERIFY(top);
    QCOMPARE(top->rowCount(), 2);

    const QHash<int, QByteArray> roles = dashboard.topCategories()->roleNames();
    QHash<QByteArray, int> byName;
    for (auto it = roles.cbegin(); it != roles.cend(); ++it)
        byName.insert(it.value(), it.key());
    QVERIFY(byName.contains("name"));
    QVERIFY(byName.contains("color"));
    QVERIFY(byName.contains("amountFormatted"));
    QVERIFY(byName.contains("share"));

    QCOMPARE(top->data(top->index(0, 0), byName.value("name")).toString(),
             QStringLiteral("Groceries"));
    QCOMPARE(top->data(top->index(0, 0), byName.value("amountFormatted")).toString(),
             CurrencyFormatter::format(Money::fromMinorUnits(30000)));
    QCOMPARE(top->data(top->index(0, 0), byName.value("share")).toDouble(), 0.75);
    QCOMPARE(top->data(top->index(1, 0), byName.value("name")).toString(),
             QStringLiteral("Bills"));
    QCOMPARE(top->data(top->index(1, 0), byName.value("share")).toDouble(), 0.25);
}

void TestDashboardViewModel::topCategories_limitedToThree()
{
    QVERIFY(addExpense(m_billsId, 40000, m_today) > 0);
    QVERIFY(addExpense(m_groceriesId, 30000, m_today) > 0);
    QVERIFY(addExpense(m_otherId, 20000, m_today) > 0);

    // A fourth category with the smallest spend must not appear.
    VERIFY_OK(CategoryRepository::add(QStringLiteral("Coffee"), QStringLiteral("#008300")));
    QVERIFY(addExpense(TestUtils::categoryId(QStringLiteral("Coffee")), 1000, m_today) > 0);

    DashboardViewModel dashboard;
    QCOMPARE(dashboard.topCategories()->rowCount(), 3);
}

void TestDashboardViewModel::refresh_onControllerSignalsUpdatesTotals()
{
    DashboardViewModel dashboard;
    QCOMPARE(dashboard.spentMinor(), qint64(0));

    ExpenseController controller;
    connect(&controller, &ExpenseController::expenseAdded,
            &dashboard, &DashboardViewModel::refresh);
    connect(&controller, &ExpenseController::expenseRemoved,
            &dashboard, &DashboardViewModel::refresh);

    QSignalSpy changedSpy(&dashboard, &DashboardViewModel::changed);
    const int id = controller.add(m_billsId, QStringLiteral("100"),
                                  QStringLiteral("Water"), m_today, QString());
    QVERIFY(id > 0);
    QCOMPARE(dashboard.spentMinor(), qint64(10000));
    QVERIFY(changedSpy.count() >= 1);

    QVERIFY(controller.remove(id));
    QCOMPARE(dashboard.spentMinor(), qint64(0));
}

QTEST_GUILESS_MAIN(TestDashboardViewModel)
#include "tst_dashboardviewmodel.moc"
