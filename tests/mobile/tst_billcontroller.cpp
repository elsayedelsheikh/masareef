#include "testutils.h"

#include "backend/billcontroller.h"
#include "storage/billrepository.h"
#include "utils/currencyformatter.h"

#include <QSignalSpy>
#include <QtTest>

class TestBillController : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void add_happyPath();
    void add_invalidAmountFails();
    void add_missingCategoryFails();
    void update_happyPath();
    void update_invalidAmountFails();
    void remove_happyPath();
    void load_populatesEditProperties();
    void load_unknownIdFails();
    void markPaid_advancesNextDueAndCreatesExpense();
    void setActive_togglesBillState();

private:
    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestBillController::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestBillController::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

void TestBillController::add_happyPath()
{
    BillController controller;
    QSignalSpy addedSpy(&controller, &BillController::billAdded);

    const int id = controller.add(m_billsId, QStringLiteral("100.00"),
                                  QStringLiteral("Electricity"), QDate(2026, 7, 10),
                                  Recurrence::Monthly, QStringLiteral("June meter"));
    QVERIFY(id > 0);
    QVERIFY(controller.lastError().isEmpty());
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.takeFirst().at(0).toInt(), id);

    const Result<RecurringBill> saved = BillRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->categoryId, m_billsId);
    QCOMPARE(saved->amount, Money::fromMinorUnits(10000));
    QCOMPARE(saved->name, QStringLiteral("Electricity"));
    QCOMPARE(saved->nextDue, QDate(2026, 7, 10));
    QCOMPARE(saved->recurrence, Recurrence::Monthly);
}

void TestBillController::add_invalidAmountFails()
{
    BillController controller;
    QCOMPARE(controller.add(m_billsId, QStringLiteral("abc"),
                            QStringLiteral("Water"), QDate(2026, 7, 10),
                            Recurrence::Monthly, QString()),
             0);
    QVERIFY(!controller.lastError().isEmpty());
}

void TestBillController::add_missingCategoryFails()
{
    BillController controller;
    QCOMPARE(controller.add(-1, QStringLiteral("10"),
                            QStringLiteral("Water"), QDate(2026, 7, 10),
                            Recurrence::Monthly, QString()),
             0);
    QVERIFY(!controller.lastError().isEmpty());
}

void TestBillController::update_happyPath()
{
    BillController controller;
    const int id = controller.add(m_billsId, QStringLiteral("100"),
                                  QStringLiteral("Electricity"), QDate(2026, 7, 10),
                                  Recurrence::Monthly, QString());
    QVERIFY(id > 0);

    QSignalSpy updatedSpy(&controller, &BillController::billUpdated);
    QVERIFY(controller.update(id, m_groceriesId, QStringLiteral("200.50"),
                              QStringLiteral("Groceries"), QDate(2026, 8, 1),
                              Recurrence::Quarterly, QStringLiteral("updated")));
    QCOMPARE(updatedSpy.count(), 1);
    QCOMPARE(updatedSpy.takeFirst().at(0).toInt(), id);

    const Result<RecurringBill> saved = BillRepository::fetch(id);
    VERIFY_OK(saved);
    QCOMPARE(saved->categoryId, m_groceriesId);
    QCOMPARE(saved->amount, Money::fromMinorUnits(20050));
}

void TestBillController::update_invalidAmountFails()
{
    BillController controller;
    const int id = controller.add(m_billsId, QStringLiteral("100"),
                                  QStringLiteral("Electricity"), QDate(2026, 7, 10),
                                  Recurrence::Monthly, QString());
    QVERIFY(id > 0);

    QSignalSpy updatedSpy(&controller, &BillController::billUpdated);
    QVERIFY(!controller.update(id, m_billsId, QStringLiteral("abc"),
                               QStringLiteral("Electricity"), QDate(2026, 7, 10),
                               Recurrence::Monthly, QString()));
    QCOMPARE(updatedSpy.count(), 0);
}

void TestBillController::remove_happyPath()
{
    BillController controller;
    const int id = controller.add(m_billsId, QStringLiteral("100"),
                                  QStringLiteral("Electricity"), QDate(2026, 7, 10),
                                  Recurrence::Monthly, QString());
    QVERIFY(id > 0);

    QSignalSpy removedSpy(&controller, &BillController::billRemoved);
    QVERIFY(controller.remove(id));
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.takeFirst().at(0).toInt(), id);
    QCOMPARE(TestUtils::countRows(QStringLiteral("recurring_bills")), 0);
}

void TestBillController::load_populatesEditProperties()
{
    BillController controller;
    const int id = controller.add(m_billsId, QStringLiteral("250.99"),
                                  QStringLiteral("Electricity"), QDate(2026, 7, 15),
                                  Recurrence::Quarterly, QStringLiteral("quarter bill"));
    QVERIFY(id > 0);

    QVERIFY(controller.load(id));
    QCOMPARE(controller.editCategoryId(), m_billsId);
    QCOMPARE(controller.editAmountText(),
             CurrencyFormatter::formatPlain(Money::fromMinorUnits(25099)));
    QCOMPARE(controller.editName(), QStringLiteral("Electricity"));
    QCOMPARE(controller.editNextDue(), QDate(2026, 7, 15));
    QCOMPARE(int(controller.editRecurrence()), int(Recurrence::Quarterly));
}

void TestBillController::load_unknownIdFails()
{
    BillController controller;
    QVERIFY(!controller.load(999999));
    QVERIFY(!controller.lastError().isEmpty());
}

void TestBillController::markPaid_advancesNextDueAndCreatesExpense()
{
    BillController controller;
    const int billId = controller.add(m_billsId, QStringLiteral("100"),
                                      QStringLiteral("Electricity"), QDate(2026, 7, 1),
                                      Recurrence::Monthly, QString());
    QVERIFY(billId > 0);

    QSignalSpy paidSpy(&controller, &BillController::billPaid);
    const QDate paidDate = QDate::currentDate();
    QVERIFY(controller.markPaid(billId, QStringLiteral("100"), paidDate,
                                QStringLiteral("Paid")));
    QCOMPARE(paidSpy.count(), 1);

    // Check bill's next_due was advanced
    const Result<RecurringBill> updated = BillRepository::fetch(billId);
    VERIFY_OK(updated);
    QVERIFY(updated->nextDue > QDate(2026, 7, 1));

    // Check expense was created
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);
}

void TestBillController::setActive_togglesBillState()
{
    BillController controller;
    const int id = controller.add(m_billsId, QStringLiteral("100"),
                                  QStringLiteral("Electricity"), QDate(2026, 7, 10),
                                  Recurrence::Monthly, QString());
    QVERIFY(id > 0);

    // Initially active
    Result<RecurringBill> bill = BillRepository::fetch(id);
    VERIFY_OK(bill);
    QVERIFY(bill->active);

    // Deactivate
    QVERIFY(controller.setActive(id, false));
    bill = BillRepository::fetch(id);
    VERIFY_OK(bill);
    QVERIFY(!bill->active);

    // Reactivate
    QVERIFY(controller.setActive(id, true));
    bill = BillRepository::fetch(id);
    VERIFY_OK(bill);
    QVERIFY(bill->active);
}

QTEST_GUILESS_MAIN(TestBillController)
#include "tst_billcontroller.moc"
