#include "testutils.h"

#include "backend/billlistmodel.h"
#include "storage/billrepository.h"
#include "utils/currencyformatter.h"

#include <QtTest>

class TestBillListModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void emptyModel_hasNoRows();
    void rowCount_reflectsActiveBills();
    void roleNames_coverAllRoles();
    void data_exposesAllRoles();
    void urgency_categorizesByDaysUntilDue();
    void remove_deletesRow();

private:
    int addBill(int categoryId, const QString& name, const QString& amount, QDate nextDue);

    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestBillListModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestBillListModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

int TestBillListModel::addBill(int categoryId, const QString& name,
                               const QString& amount, QDate nextDue)
{
    RecurringBill bill;
    bill.categoryId = categoryId;
    bill.name = name;
    bill.amount = CurrencyFormatter::parse(amount).value_or(Money());
    bill.nextDue = nextDue;
    bill.recurrence = Recurrence::Monthly;
    const Result<int> added = BillRepository::add(bill);
    return added ? *added : -1;
}

void TestBillListModel::emptyModel_hasNoRows()
{
    BillListModel model;
    QCOMPARE(model.rowCount(), 0);
}

void TestBillListModel::rowCount_reflectsActiveBills()
{
    addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
            QDate(2026, 7, 10));
    addBill(m_groceriesId, QStringLiteral("Groceries"), QStringLiteral("50"),
            QDate(2026, 7, 15));

    BillListModel model;
    QCOMPARE(model.rowCount(), 2);

    // Deactivate one bill
    QVERIFY(BillRepository::setActive(1, false));
    model.refresh();
    QCOMPARE(model.rowCount(), 1); // Only active bills shown
}

void TestBillListModel::roleNames_coverAllRoles()
{
    BillListModel model;
    const auto names = model.roleNames();
    QVERIFY(names.contains(BillListModel::BillIdRole));
    QVERIFY(names.contains(BillListModel::CategoryIdRole));
    QVERIFY(names.contains(BillListModel::CategoryNameRole));
    QVERIFY(names.contains(BillListModel::NameRole));
    QVERIFY(names.contains(BillListModel::AmountFormattedRole));
    QVERIFY(names.contains(BillListModel::NextDueRole));
    QVERIFY(names.contains(BillListModel::UrgencyRole));
}

void TestBillListModel::data_exposesAllRoles()
{
    const int id = addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
                           QDate(2026, 7, 10));
    QVERIFY(id > 0);

    BillListModel model;
    QCOMPARE(model.rowCount(), 1);

    const QModelIndex idx = model.index(0, 0);
    QCOMPARE(model.data(idx, BillListModel::BillIdRole).toInt(), id);
    QCOMPARE(model.data(idx, BillListModel::CategoryIdRole).toInt(), m_billsId);
    QCOMPARE(model.data(idx, BillListModel::NameRole).toString(), QStringLiteral("Electric"));
    QCOMPARE(model.data(idx, BillListModel::AmountFormattedRole).toString(),
             CurrencyFormatter::format(Money::fromMinorUnits(10000)));
    QCOMPARE(model.data(idx, BillListModel::NextDueRole).toDate(), QDate(2026, 7, 10));
}

void TestBillListModel::urgency_categorizesByDaysUntilDue()
{
    // Overdue
    addBill(m_billsId, QStringLiteral("Overdue"), QStringLiteral("100"),
            QDate::currentDate().addDays(-5));
    // Due soon (≤7d)
    addBill(m_billsId, QStringLiteral("Soon"), QStringLiteral("100"),
            QDate::currentDate().addDays(3));
    // Due later (≤30d)
    addBill(m_billsId, QStringLiteral("Later"), QStringLiteral("100"),
            QDate::currentDate().addDays(20));
    // Far future
    addBill(m_billsId, QStringLiteral("Future"), QStringLiteral("100"),
            QDate::currentDate().addDays(60));

    BillListModel model;
    QCOMPARE(model.rowCount(), 4);

    QCOMPARE(model.data(model.index(0, 0), BillListModel::UrgencyRole).toString(),
             QStringLiteral("overdue"));
    QCOMPARE(model.data(model.index(1, 0), BillListModel::UrgencyRole).toString(),
             QStringLiteral("due"));
    QCOMPARE(model.data(model.index(2, 0), BillListModel::UrgencyRole).toString(),
             QStringLiteral("upcoming"));
    QCOMPARE(model.data(model.index(3, 0), BillListModel::UrgencyRole).toString(),
             QStringLiteral("later"));
}

void TestBillListModel::remove_deletesRow()
{
    const int id = addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
                           QDate(2026, 7, 10));
    QVERIFY(id > 0);

    BillListModel model;
    QCOMPARE(model.rowCount(), 1);

    QVERIFY(model.removeAt(0));
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("recurring_bills")), 0);
}

QTEST_GUILESS_MAIN(TestBillListModel)
#include "tst_billlistmodel.moc"
