#include "testutils.h"

#include "storage/budgetrepository.h"

#include <QtTest>

class TestBudgetRepository : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void overallBudget_defaultsToUnset();
    void overallBudget_setReplaceClear();
    void categoryBudgets_setReplaceClear();
    void categoryAndOverall_areIndependent();

private:
    int m_billsId = -1;
};

void TestBudgetRepository::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestBudgetRepository::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    QVERIFY(m_billsId > 0);
}

void TestBudgetRepository::overallBudget_defaultsToUnset()
{
    QVERIFY(!BudgetRepository::overallBudget().has_value());
    QVERIFY(BudgetRepository::categoryBudgets().isEmpty());
}

void TestBudgetRepository::overallBudget_setReplaceClear()
{
    VERIFY_OK(BudgetRepository::setOverallBudget(Money::fromMinorUnits(500000)));
    QCOMPARE(BudgetRepository::overallBudget(),
             std::optional(Money::fromMinorUnits(500000)));

    // Setting again replaces rather than accumulates
    VERIFY_OK(BudgetRepository::setOverallBudget(Money::fromMinorUnits(750000)));
    QCOMPARE(BudgetRepository::overallBudget(),
             std::optional(Money::fromMinorUnits(750000)));

    VERIFY_OK(BudgetRepository::setOverallBudget(std::nullopt));
    QVERIFY(!BudgetRepository::overallBudget().has_value());
}

void TestBudgetRepository::categoryBudgets_setReplaceClear()
{
    VERIFY_OK(BudgetRepository::setCategoryBudget(m_billsId,
                                                  Money::fromMinorUnits(120000)));
    QHash<int, Money> budgets = BudgetRepository::categoryBudgets();
    QCOMPARE(budgets.size(), 1);
    QCOMPARE(budgets.value(m_billsId), Money::fromMinorUnits(120000));

    VERIFY_OK(BudgetRepository::setCategoryBudget(m_billsId,
                                                  Money::fromMinorUnits(90000)));
    budgets = BudgetRepository::categoryBudgets();
    QCOMPARE(budgets.size(), 1);
    QCOMPARE(budgets.value(m_billsId), Money::fromMinorUnits(90000));

    VERIFY_OK(BudgetRepository::setCategoryBudget(m_billsId, std::nullopt));
    QVERIFY(BudgetRepository::categoryBudgets().isEmpty());
}

void TestBudgetRepository::categoryAndOverall_areIndependent()
{
    VERIFY_OK(BudgetRepository::setOverallBudget(Money::fromMinorUnits(500000)));
    VERIFY_OK(BudgetRepository::setCategoryBudget(m_billsId,
                                                  Money::fromMinorUnits(120000)));

    // Clearing one leaves the other untouched
    VERIFY_OK(BudgetRepository::setCategoryBudget(m_billsId, std::nullopt));
    QCOMPARE(BudgetRepository::overallBudget(),
             std::optional(Money::fromMinorUnits(500000)));

    VERIFY_OK(BudgetRepository::setCategoryBudget(m_billsId,
                                                  Money::fromMinorUnits(120000)));
    VERIFY_OK(BudgetRepository::setOverallBudget(std::nullopt));
    QCOMPARE(BudgetRepository::categoryBudgets().value(m_billsId),
             Money::fromMinorUnits(120000));
}

QTEST_GUILESS_MAIN(TestBudgetRepository)
#include "tst_budgetrepository.moc"
