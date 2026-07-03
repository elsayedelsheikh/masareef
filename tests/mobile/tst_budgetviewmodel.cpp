#include "testutils.h"

#include "backend/budgetviewmodel.h"
#include "backend/expensecontroller.h"
#include "storage/budgetrepository.h"
#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"

#include <QSignalSpy>
#include <QtTest>

class TestBudgetViewModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void listsAllCategories();
    void roleNames_coverAllRoles();
    void setOverallBudget_parsesAndStores();
    void setOverallBudget_emptyTextClears();
    void setOverallBudget_garbageFailsWithError();
    void setCategoryBudget_roundTrip();
    void setCategoryBudget_emptyTextClears();
    void setCategoryBudget_garbageFailsWithError();
    void progress_reflectsMonthSpend();
    void refresh_onControllerSignalUpdatesSpend();

private:
    int rowOf(const BudgetViewModel& model, int categoryId);

    int m_billsId = -1;
    QDate m_today;
};

void TestBudgetViewModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestBudgetViewModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    QVERIFY(m_billsId > 0);
    m_today = QDate::currentDate();
}

int TestBudgetViewModel::rowOf(const BudgetViewModel& model, int categoryId)
{
    for (int row = 0; row < model.rowCount(); ++row) {
        if (model.data(model.index(row, 0), BudgetViewModel::CategoryIdRole).toInt()
            == categoryId)
            return row;
    }
    return -1;
}

void TestBudgetViewModel::listsAllCategories()
{
    BudgetViewModel model;
    QCOMPARE(model.rowCount(), int(CategoryRepository::all().size()));
    QVERIFY(rowOf(model, m_billsId) >= 0);
}

void TestBudgetViewModel::roleNames_coverAllRoles()
{
    BudgetViewModel model;
    const QHash<int, QByteArray> roles = model.roleNames();
    const QList<QByteArray> expected = {
        "categoryId", "name", "color", "budgetMinor", "hasBudget",
        "spentMinor", "progress", "budgetText",
    };
    for (const QByteArray& role : expected)
        QVERIFY2(roles.values().contains(role), role.constData());
}

void TestBudgetViewModel::setOverallBudget_parsesAndStores()
{
    BudgetViewModel model;
    QCOMPARE(model.hasOverallBudget(), false);

    QSignalSpy changedSpy(&model, &BudgetViewModel::overallChanged);
    QVERIFY(model.setOverallBudget(QStringLiteral("500")));
    QVERIFY(model.lastError().isEmpty());
    QCOMPARE(model.hasOverallBudget(), true);
    QCOMPARE(model.overallBudgetMinor(), qint64(50000));
    QCOMPARE(model.overallBudgetText(),
             CurrencyFormatter::formatPlain(Money::fromMinorUnits(50000)));
    QVERIFY(changedSpy.count() >= 1);

    const std::optional<Money> stored = BudgetRepository::overallBudget();
    QVERIFY(stored.has_value());
    QCOMPARE(*stored, Money::fromMinorUnits(50000));
}

void TestBudgetViewModel::setOverallBudget_emptyTextClears()
{
    BudgetViewModel model;
    QVERIFY(model.setOverallBudget(QStringLiteral("500")));
    QVERIFY(model.setOverallBudget(QString()));
    QCOMPARE(model.hasOverallBudget(), false);
    QCOMPARE(model.overallBudgetMinor(), qint64(0));
    QVERIFY(model.overallBudgetText().isEmpty());
    QVERIFY(!BudgetRepository::overallBudget().has_value());
}

void TestBudgetViewModel::setOverallBudget_garbageFailsWithError()
{
    BudgetViewModel model;
    QVERIFY(model.setOverallBudget(QStringLiteral("500")));
    QVERIFY(!model.setOverallBudget(QStringLiteral("abc")));
    QVERIFY(!model.lastError().isEmpty());
    // Budget unchanged
    QCOMPARE(model.overallBudgetMinor(), qint64(50000));
}

void TestBudgetViewModel::setCategoryBudget_roundTrip()
{
    BudgetViewModel model;
    QVERIFY(model.setCategoryBudget(m_billsId, QStringLiteral("200")));

    const int row = rowOf(model, m_billsId);
    QVERIFY(row >= 0);
    const QModelIndex index = model.index(row, 0);
    QCOMPARE(model.data(index, BudgetViewModel::BudgetMinorRole).toLongLong(),
             qint64(20000));
    QCOMPARE(model.data(index, BudgetViewModel::HasBudgetRole).toBool(), true);
    QCOMPARE(model.data(index, BudgetViewModel::BudgetTextRole).toString(),
             CurrencyFormatter::formatPlain(Money::fromMinorUnits(20000)));

    QCOMPARE(BudgetRepository::categoryBudgets().value(m_billsId),
             Money::fromMinorUnits(20000));
}

void TestBudgetViewModel::setCategoryBudget_emptyTextClears()
{
    BudgetViewModel model;
    QVERIFY(model.setCategoryBudget(m_billsId, QStringLiteral("200")));
    QVERIFY(model.setCategoryBudget(m_billsId, QStringLiteral("  ")));

    const QModelIndex index = model.index(rowOf(model, m_billsId), 0);
    QCOMPARE(model.data(index, BudgetViewModel::HasBudgetRole).toBool(), false);
    QCOMPARE(model.data(index, BudgetViewModel::BudgetMinorRole).toLongLong(), qint64(0));
    QVERIFY(model.data(index, BudgetViewModel::BudgetTextRole).toString().isEmpty());
    QVERIFY(!BudgetRepository::categoryBudgets().contains(m_billsId));
}

void TestBudgetViewModel::setCategoryBudget_garbageFailsWithError()
{
    BudgetViewModel model;
    QVERIFY(!model.setCategoryBudget(m_billsId, QStringLiteral("1.2.3")));
    QVERIFY(!model.lastError().isEmpty());
    QVERIFY(!BudgetRepository::categoryBudgets().contains(m_billsId));
}

void TestBudgetViewModel::progress_reflectsMonthSpend()
{
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(5000),
                                       .description = QStringLiteral("Water"),
                                       .date = m_today }));

    BudgetViewModel model;
    QVERIFY(model.setOverallBudget(QStringLiteral("200")));
    QVERIFY(model.setCategoryBudget(m_billsId, QStringLiteral("100")));

    QCOMPARE(model.overallSpentMinor(), qint64(5000));
    QCOMPARE(model.overallProgress(), 0.25);

    const QModelIndex index = model.index(rowOf(model, m_billsId), 0);
    QCOMPARE(model.data(index, BudgetViewModel::SpentMinorRole).toLongLong(),
             qint64(5000));
    QCOMPARE(model.data(index, BudgetViewModel::ProgressRole).toDouble(), 0.5);
}

void TestBudgetViewModel::refresh_onControllerSignalUpdatesSpend()
{
    BudgetViewModel model;
    QVERIFY(model.setOverallBudget(QStringLiteral("100")));
    QCOMPARE(model.overallSpentMinor(), qint64(0));

    ExpenseController controller;
    connect(&controller, &ExpenseController::expenseAdded,
            &model, &BudgetViewModel::refresh);

    const int id = controller.add(m_billsId, QStringLiteral("25"),
                                  QStringLiteral("Water"), m_today, QString());
    QVERIFY(id > 0);
    QCOMPARE(model.overallSpentMinor(), qint64(2500));
    QCOMPARE(model.overallProgress(), 0.25);
}

QTEST_GUILESS_MAIN(TestBudgetViewModel)
#include "tst_budgetviewmodel.moc"
