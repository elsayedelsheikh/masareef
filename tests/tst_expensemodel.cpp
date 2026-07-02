#include "testutils.h"

#include "models/expensemodel.h"
#include "utils/currencyformatter.h"

#include <QtTest>

class TestExpenseModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void addAndFetch_roundTrip();
    void updateExpense_changesFields();
    void deleteExpense_removesRow();
    void model_dateRangeFilter();
    void model_categoryFilter();
    void model_formatsAmountColumn();
    void totalsByCategory_currentRange();
    void monthlyTotals_fillsMissingMonths();

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

void TestExpenseModel::addAndFetch_roundTrip()
{
    QString error;
    QVERIFY2(ExpenseModel::addExpense(m_billsId, 25099, QStringLiteral("Electricity"),
                                      QDate(2026, 6, 15), QStringLiteral("June meter"),
                                      QVariant(), &error),
             qPrintable(error));
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);

    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("SELECT id FROM expenses")));
    QVERIFY(query.next());
    const int id = query.value(0).toInt();

    ExpenseModel::Expense expense;
    QVERIFY(ExpenseModel::fetchExpense(id, &expense, &error));
    QCOMPARE(expense.categoryId, m_billsId);
    QCOMPARE(expense.amount, qint64(25099));
    QCOMPARE(expense.description, QStringLiteral("Electricity"));
    QCOMPARE(expense.date, QDate(2026, 6, 15));
    QCOMPARE(expense.notes, QStringLiteral("June meter"));
}

void TestExpenseModel::updateExpense_changesFields()
{
    QString error;
    QVERIFY(ExpenseModel::addExpense(m_billsId, 1000, QStringLiteral("Water"),
                                     QDate(2026, 6, 1), QString(), QVariant(), &error));
    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("SELECT id FROM expenses")));
    QVERIFY(query.next());
    const int id = query.value(0).toInt();

    QVERIFY2(ExpenseModel::updateExpense(id, m_groceriesId, 2000, QStringLiteral("Market"),
                                         QDate(2026, 6, 2), QStringLiteral("updated"), &error),
             qPrintable(error));

    ExpenseModel::Expense expense;
    QVERIFY(ExpenseModel::fetchExpense(id, &expense, &error));
    QCOMPARE(expense.categoryId, m_groceriesId);
    QCOMPARE(expense.amount, qint64(2000));
    QCOMPARE(expense.description, QStringLiteral("Market"));
    QCOMPARE(expense.date, QDate(2026, 6, 2));
    QCOMPARE(expense.notes, QStringLiteral("updated"));
}

void TestExpenseModel::deleteExpense_removesRow()
{
    QString error;
    QVERIFY(ExpenseModel::addExpense(m_billsId, 1000, QString(), QDate(2026, 6, 1),
                                     QString(), QVariant(), &error));
    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("SELECT id FROM expenses")));
    QVERIFY(query.next());
    QVERIFY2(ExpenseModel::deleteExpense(query.value(0).toInt(), &error), qPrintable(error));
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseModel::model_dateRangeFilter()
{
    QString error;
    QVERIFY(ExpenseModel::addExpense(m_billsId, 100, QStringLiteral("May"),
                                     QDate(2026, 5, 20), QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_billsId, 200, QStringLiteral("June A"),
                                     QDate(2026, 6, 1), QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_groceriesId, 300, QStringLiteral("June B"),
                                     QDate(2026, 6, 30), QString(), QVariant(), &error));

    ExpenseModel model;
    model.setDateRange(QDate(2026, 6, 1), QDate(2026, 6, 30));
    model.refresh();
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.filteredTotal(), qint64(500));
}

void TestExpenseModel::model_categoryFilter()
{
    QString error;
    QVERIFY(ExpenseModel::addExpense(m_billsId, 200, QStringLiteral("Bill"),
                                     QDate(2026, 6, 1), QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_groceriesId, 300, QStringLiteral("Food"),
                                     QDate(2026, 6, 2), QString(), QVariant(), &error));

    ExpenseModel model;
    model.setDateRange(QDate(2026, 6, 1), QDate(2026, 6, 30));
    model.setCategoryFilter(m_groceriesId);
    model.refresh();
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.filteredTotal(), qint64(300));
    QCOMPARE(model.data(model.index(0, ExpenseModel::ColCategory)).toString(),
             QStringLiteral("Groceries"));
}

void TestExpenseModel::model_formatsAmountColumn()
{
    QString error;
    QVERIFY(ExpenseModel::addExpense(m_billsId, 10050, QStringLiteral("X"),
                                     QDate(2026, 6, 1), QString(), QVariant(), &error));
    ExpenseModel model;
    model.setDateRange(QDate(2026, 6, 1), QDate(2026, 6, 30));
    model.refresh();
    QCOMPARE(model.data(model.index(0, ExpenseModel::ColAmount)).toString(),
             CurrencyFormatter::format(10050));
    QCOMPARE(model.expenseIdAt(0) > 0, true);
}

void TestExpenseModel::totalsByCategory_currentRange()
{
    QString error;
    QVERIFY(ExpenseModel::addExpense(m_billsId, 100, QString(), QDate(2026, 6, 1),
                                     QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_billsId, 150, QString(), QDate(2026, 6, 5),
                                     QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_groceriesId, 300, QString(), QDate(2026, 6, 9),
                                     QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_groceriesId, 999, QString(), QDate(2026, 7, 1),
                                     QString(), QVariant(), &error));

    const auto totals =
        ExpenseModel::totalsByCategory(QDate(2026, 6, 1), QDate(2026, 6, 30));
    QCOMPARE(totals.size(), 2);
    // Sorted by total, descending
    QCOMPARE(totals.at(0).name, QStringLiteral("Groceries"));
    QCOMPARE(totals.at(0).total, qint64(300));
    QCOMPARE(totals.at(1).name, QStringLiteral("Bills"));
    QCOMPARE(totals.at(1).total, qint64(250));
    QVERIFY(!totals.at(0).color.isEmpty());

    QCOMPARE(ExpenseModel::totalBetween(QDate(2026, 6, 1), QDate(2026, 6, 30)), qint64(550));
    QCOMPARE(ExpenseModel::totalBetween(QDate(2026, 6, 1), QDate(2026, 6, 30), m_billsId),
             qint64(250));
}

void TestExpenseModel::monthlyTotals_fillsMissingMonths()
{
    QString error;
    const QDate now = QDate::currentDate();
    const QDate thisMonth(now.year(), now.month(), 1);
    const QDate threeBack = thisMonth.addMonths(-3);
    QVERIFY(ExpenseModel::addExpense(m_billsId, 400, QString(), thisMonth.addDays(5),
                                     QString(), QVariant(), &error));
    QVERIFY(ExpenseModel::addExpense(m_billsId, 700, QString(), threeBack.addDays(2),
                                     QString(), QVariant(), &error));

    const auto months = ExpenseModel::monthlyTotals(12);
    QCOMPARE(months.size(), 12);
    QCOMPARE(months.last().month, thisMonth);
    QCOMPARE(months.last().total, qint64(400));
    QCOMPARE(months.at(8).month, threeBack);
    QCOMPARE(months.at(8).total, qint64(700));
    QCOMPARE(months.at(7).total, qint64(0));
}

QTEST_GUILESS_MAIN(TestExpenseModel)
#include "tst_expensemodel.moc"
