#include "testutils.h"

#include "backend/expensecontroller.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"

#include <QSignalSpy>
#include <QtTest>

class TestExpenseController : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void add_happyPath();
    void add_trimsDescriptionAndNotes();
    void add_invalidAmountFails_data();
    void add_invalidAmountFails();
    void add_missingCategoryFails();
    void add_unknownCategoryFails();
    void add_invalidDateFails();
    void update_happyPath();
    void update_invalidAmountFails();
    void remove_happyPath();
    void restore_reAddsRemovedExpense();
    void load_populatesEditProperties();
    void load_unknownIdFails();
    void removeMany_deletesMultipleExpenses();

private:
    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestExpenseController::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestExpenseController::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

void TestExpenseController::add_happyPath()
{
    ExpenseController controller;
    QSignalSpy addedSpy(&controller, &ExpenseController::expenseAdded);

    const int id = controller.add(m_billsId, QStringLiteral("250.99"),
                                  QStringLiteral("Electricity"), QDate(2026, 6, 15),
                                  QStringLiteral("June meter"));
    QVERIFY(id > 0);
    QVERIFY(controller.lastError().isEmpty());
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.takeFirst().at(0).toInt(), id);

    const Result<Expense> saved = ExpenseRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->categoryId, m_billsId);
    QCOMPARE(saved->amount, Money::fromMinorUnits(25099));
    QCOMPARE(saved->description, QStringLiteral("Electricity"));
    QCOMPARE(saved->date, QDate(2026, 6, 15));
    QCOMPARE(saved->notes, QStringLiteral("June meter"));
}

void TestExpenseController::add_trimsDescriptionAndNotes()
{
    ExpenseController controller;
    const int id = controller.add(m_billsId, QStringLiteral("10"),
                                  QStringLiteral("  Water  "), QDate(2026, 6, 1),
                                  QStringLiteral("  note  "));
    QVERIFY(id > 0);

    const Result<Expense> saved = ExpenseRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->description, QStringLiteral("Water"));
    QCOMPARE(saved->notes, QStringLiteral("note"));
}

void TestExpenseController::add_invalidAmountFails_data()
{
    QTest::addColumn<QString>("amountText");
    QTest::newRow("empty") << QString();
    QTest::newRow("letters") << QStringLiteral("abc");
    QTest::newRow("negative") << QStringLiteral("-5");
    QTest::newRow("zero") << QStringLiteral("0");
    QTest::newRow("two dots") << QStringLiteral("1.2.3");
}

void TestExpenseController::add_invalidAmountFails()
{
    QFETCH(QString, amountText);

    ExpenseController controller;
    QSignalSpy addedSpy(&controller, &ExpenseController::expenseAdded);

    QCOMPARE(controller.add(m_billsId, amountText, QStringLiteral("Water"),
                            QDate(2026, 6, 1), QString()),
             0);
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseController::add_missingCategoryFails()
{
    ExpenseController controller;
    QCOMPARE(controller.add(-1, QStringLiteral("10"), QStringLiteral("Water"),
                            QDate(2026, 6, 1), QString()),
             0);
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseController::add_unknownCategoryFails()
{
    ExpenseController controller;
    QCOMPARE(controller.add(999999, QStringLiteral("10"), QStringLiteral("Water"),
                            QDate(2026, 6, 1), QString()),
             0);
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseController::add_invalidDateFails()
{
    ExpenseController controller;
    QCOMPARE(controller.add(m_billsId, QStringLiteral("10"), QStringLiteral("Water"),
                            QDate(), QString()),
             0);
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseController::update_happyPath()
{
    ExpenseController controller;
    const int id = controller.add(m_billsId, QStringLiteral("10"),
                                  QStringLiteral("Water"), QDate(2026, 6, 1),
                                  QString());
    QVERIFY(id > 0);

    QSignalSpy updatedSpy(&controller, &ExpenseController::expenseUpdated);
    QVERIFY(controller.update(id, m_groceriesId, QStringLiteral("20.50"),
                              QStringLiteral("Bread"), QDate(2026, 6, 2),
                              QStringLiteral("bakery")));
    QCOMPARE(updatedSpy.count(), 1);
    QCOMPARE(updatedSpy.takeFirst().at(0).toInt(), id);

    const Result<Expense> saved = ExpenseRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->categoryId, m_groceriesId);
    QCOMPARE(saved->amount, Money::fromMinorUnits(2050));
    QCOMPARE(saved->description, QStringLiteral("Bread"));
    QCOMPARE(saved->date, QDate(2026, 6, 2));
    QCOMPARE(saved->notes, QStringLiteral("bakery"));
}

void TestExpenseController::update_invalidAmountFails()
{
    ExpenseController controller;
    const int id = controller.add(m_billsId, QStringLiteral("10"),
                                  QStringLiteral("Water"), QDate(2026, 6, 1),
                                  QString());
    QVERIFY(id > 0);

    QSignalSpy updatedSpy(&controller, &ExpenseController::expenseUpdated);
    QVERIFY(!controller.update(id, m_billsId, QStringLiteral("abc"),
                               QStringLiteral("Water"), QDate(2026, 6, 1), QString()));
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(updatedSpy.count(), 0);

    const Result<Expense> saved = ExpenseRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->amount, Money::fromMinorUnits(1000)); // unchanged
}

void TestExpenseController::remove_happyPath()
{
    ExpenseController controller;
    const int id = controller.add(m_billsId, QStringLiteral("10"),
                                  QStringLiteral("Water"), QDate(2026, 6, 1),
                                  QString());
    QVERIFY(id > 0);

    QSignalSpy removedSpy(&controller, &ExpenseController::expenseRemoved);
    QVERIFY(controller.remove(id));
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.takeFirst().at(0).toInt(), id);
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 0);
}

void TestExpenseController::restore_reAddsRemovedExpense()
{
    // Undo of a swipe-delete: the row data comes back from the model roles
    // (minor units, not text), so restore takes the amount directly.
    ExpenseController controller;
    QSignalSpy addedSpy(&controller, &ExpenseController::expenseAdded);

    const int id = controller.restore(m_billsId, 25099, QStringLiteral("Electricity"),
                                      QDate(2026, 6, 15), QStringLiteral("June meter"));
    QVERIFY(id > 0);
    QCOMPARE(addedSpy.count(), 1);

    const Result<Expense> saved = ExpenseRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->amount, Money::fromMinorUnits(25099));
    QCOMPARE(saved->description, QStringLiteral("Electricity"));
    QCOMPARE(saved->date, QDate(2026, 6, 15));
    QCOMPARE(saved->notes, QStringLiteral("June meter"));
}

void TestExpenseController::load_populatesEditProperties()
{
    ExpenseController controller;
    const int id = controller.add(m_billsId, QStringLiteral("250.99"),
                                  QStringLiteral("Electricity"), QDate(2026, 6, 15),
                                  QStringLiteral("June meter"));
    QVERIFY(id > 0);

    QVERIFY(controller.load(id));
    QCOMPARE(controller.editCategoryId(), m_billsId);
    QCOMPARE(controller.editAmountText(),
             CurrencyFormatter::formatPlain(Money::fromMinorUnits(25099)));
    QCOMPARE(controller.editDescription(), QStringLiteral("Electricity"));
    QCOMPARE(controller.editDate(), QDate(2026, 6, 15));
    QCOMPARE(controller.editNotes(), QStringLiteral("June meter"));
}

void TestExpenseController::load_unknownIdFails()
{
    ExpenseController controller;
    QVERIFY(!controller.load(999999));
    QVERIFY(!controller.lastError().isEmpty());
}

void TestExpenseController::removeMany_deletesMultipleExpenses()
{
    ExpenseController controller;
    const int id1 = controller.add(m_billsId, QStringLiteral("10"),
                                   QStringLiteral("Bill1"), QDate(2026, 6, 1),
                                   QString());
    const int id2 = controller.add(m_groceriesId, QStringLiteral("20"),
                                   QStringLiteral("Groceries"), QDate(2026, 6, 2),
                                   QString());
    const int id3 = controller.add(m_billsId, QStringLiteral("30"),
                                   QStringLiteral("Bill2"), QDate(2026, 6, 3),
                                   QString());
    QVERIFY(id1 > 0 && id2 > 0 && id3 > 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 3);

    QSignalSpy removedSpy(&controller, &ExpenseController::expenseRemoved);
    QVERIFY(controller.removeMany(QList<int>{ id1, id3 }));
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);

    // Verify the remaining expense is correct
    const Result<Expense> saved = ExpenseRepository::fetch(id2);
    VERIFY_OK(saved);
    QCOMPARE(saved->description, QStringLiteral("Groceries"));
}

QTEST_GUILESS_MAIN(TestExpenseController)
#include "tst_expensecontroller.moc"
