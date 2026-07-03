#include "testutils.h"

#include "models/expensemodel.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"

#include <QtTest>

// The model is a thin adapter over ExpenseRepository::makeFilteredQuery;
// only the presentation concerns live here (query wiring, amount
// formatting, id lookup). The querying itself is covered by
// tst_expenserepository.
class TestExpenseModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void refresh_appliesDateRange();
    void refresh_appliesCategoryFilter();
    void data_formatsAmountColumn();

private:
    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestExpenseModel::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestExpenseModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

void TestExpenseModel::refresh_appliesDateRange()
{
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(100),
                                       .description = QStringLiteral("May"),
                                       .date = QDate(2026, 5, 20) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(200),
                                       .description = QStringLiteral("June A"),
                                       .date = QDate(2026, 6, 1) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_groceriesId,
                                       .amount = Money::fromMinorUnits(300),
                                       .description = QStringLiteral("June B"),
                                       .date = QDate(2026, 6, 30) }));

    ExpenseModel model;
    model.setFilter({ .from = QDate(2026, 6, 1), .to = QDate(2026, 6, 30) });
    model.refresh();
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.filteredTotal(), Money::fromMinorUnits(500));
}

void TestExpenseModel::refresh_appliesCategoryFilter()
{
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(200),
                                       .description = QStringLiteral("Bill"),
                                       .date = QDate(2026, 6, 1) }));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_groceriesId,
                                       .amount = Money::fromMinorUnits(300),
                                       .description = QStringLiteral("Food"),
                                       .date = QDate(2026, 6, 2) }));

    ExpenseModel model;
    model.setFilter({ .from = QDate(2026, 6, 1),
                      .to = QDate(2026, 6, 30),
                      .categoryId = m_groceriesId });
    model.refresh();
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.filteredTotal(), Money::fromMinorUnits(300));
    QCOMPARE(model.data(model.index(0, ExpenseModel::ColCategory)).toString(),
             QStringLiteral("Groceries"));
}

void TestExpenseModel::data_formatsAmountColumn()
{
    VERIFY_OK(ExpenseRepository::add({ .categoryId = m_billsId,
                                       .amount = Money::fromMinorUnits(10050),
                                       .description = QStringLiteral("X"),
                                       .date = QDate(2026, 6, 1) }));
    ExpenseModel model;
    model.setFilter({ .from = QDate(2026, 6, 1), .to = QDate(2026, 6, 30) });
    model.refresh();
    QCOMPARE(model.data(model.index(0, ExpenseModel::ColAmount)).toString(),
             CurrencyFormatter::format(Money::fromMinorUnits(10050)));
    QVERIFY(model.expenseIdAt(0) > 0);
}

QTEST_GUILESS_MAIN(TestExpenseModel)
#include "tst_expensemodel.moc"
