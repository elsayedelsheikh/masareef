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

    void showPaused_revealsDeactivatedBills();
    void pausedBills_areGroupedAtTheEnd();
    void pausedBills_urgencyIsPaused();
    void activeCount_ignoresPausedBills();
    void monthlyTotal_normalizesEveryRecurrence();
    void monthlyTotal_ignoresPausedBills();
    void dueLabel_describesTheDueDate();
    void recurrenceLabel_isPresentForEveryRow();

private:
    int addBill(int categoryId, const QString& name, const QString& amount, QDate nextDue,
                Recurrence recurrence = Recurrence::Monthly);

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
                               const QString& amount, QDate nextDue,
                               Recurrence recurrence)
{
    RecurringBill bill;
    bill.categoryId = categoryId;
    bill.name = name;
    bill.amount = CurrencyFormatter::parse(amount).value_or(Money());
    bill.nextDue = nextDue;
    bill.recurrence = recurrence;
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
    const int bill1Id = addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
                                 QDate(2026, 7, 10));
    addBill(m_groceriesId, QStringLiteral("Groceries"), QStringLiteral("50"),
            QDate(2026, 7, 15));

    BillListModel model;
    QCOMPARE(model.rowCount(), 2);

    // Deactivate one bill
    QVERIFY(BillRepository::setActive(bill1Id, false));
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

void TestBillListModel::showPaused_revealsDeactivatedBills()
{
    const int id = addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
                           QDate::currentDate().addDays(5));
    QVERIFY(id > 0);
    QVERIFY(BillRepository::setActive(id, false));

    BillListModel model;
    QCOMPARE(model.rowCount(), 0); // paused bills are hidden by default

    model.setShowPaused(true);
    QCOMPARE(model.rowCount(), 1);
    QVERIFY(model.data(model.index(0, 0), BillListModel::ActiveRole).isValid());
    QVERIFY(!model.data(model.index(0, 0), BillListModel::ActiveRole).toBool());

    model.setShowPaused(false);
    QCOMPARE(model.rowCount(), 0);
}

void TestBillListModel::pausedBills_areGroupedAtTheEnd()
{
    // The repository orders by due date, so a paused bill due soon would
    // land in the middle of the active ones — and a sectioned list would
    // then print the "Paused" header more than once.
    const int paused = addBill(m_billsId, QStringLiteral("Gym"), QStringLiteral("100"),
                               QDate::currentDate().addDays(2));
    addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
            QDate::currentDate().addDays(1));
    addBill(m_billsId, QStringLiteral("Rent"), QStringLiteral("100"),
            QDate::currentDate().addDays(3));
    QVERIFY(paused > 0);
    QVERIFY(BillRepository::setActive(paused, false));

    BillListModel model;
    model.setShowPaused(true);
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.data(model.index(0, 0), BillListModel::NameRole).toString(),
             QStringLiteral("Electric"));
    QCOMPARE(model.data(model.index(1, 0), BillListModel::NameRole).toString(),
             QStringLiteral("Rent"));
    QCOMPARE(model.data(model.index(2, 0), BillListModel::NameRole).toString(),
             QStringLiteral("Gym"));
}

void TestBillListModel::pausedBills_urgencyIsPaused()
{
    // Overdue *and* paused reads as paused: it is not chasing anything.
    const int id = addBill(m_billsId, QStringLiteral("Gym"), QStringLiteral("100"),
                           QDate::currentDate().addDays(-30));
    QVERIFY(id > 0);
    QVERIFY(BillRepository::setActive(id, false));

    BillListModel model;
    model.setShowPaused(true);
    QCOMPARE(model.data(model.index(0, 0), BillListModel::UrgencyRole).toString(),
             QStringLiteral("paused"));
}

void TestBillListModel::activeCount_ignoresPausedBills()
{
    const int paused = addBill(m_billsId, QStringLiteral("Gym"), QStringLiteral("100"),
                               QDate::currentDate().addDays(5));
    addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("100"),
            QDate::currentDate().addDays(6));
    QVERIFY(paused > 0);
    QVERIFY(BillRepository::setActive(paused, false));

    BillListModel model;
    model.setShowPaused(true);
    QCOMPARE(model.count(), 2);
    QCOMPARE(model.activeCount(), 1);
}

void TestBillListModel::monthlyTotal_normalizesEveryRecurrence()
{
    addBill(m_billsId, QStringLiteral("Monthly"), QStringLiteral("100"),
            QDate::currentDate().addDays(5), Recurrence::Monthly);
    addBill(m_billsId, QStringLiteral("Quarterly"), QStringLiteral("300"),
            QDate::currentDate().addDays(6), Recurrence::Quarterly);
    addBill(m_billsId, QStringLiteral("Yearly"), QStringLiteral("1200"),
            QDate::currentDate().addDays(7), Recurrence::Yearly);

    // 100 + 300/3 + 1200/12 = 300 a month
    BillListModel model;
    QCOMPARE(model.monthlyTotalFormatted(),
             CurrencyFormatter::format(Money::fromMinorUnits(30000)));
}

void TestBillListModel::monthlyTotal_ignoresPausedBills()
{
    const int paused = addBill(m_billsId, QStringLiteral("Gym"), QStringLiteral("100"),
                               QDate::currentDate().addDays(5));
    addBill(m_billsId, QStringLiteral("Electric"), QStringLiteral("250"),
            QDate::currentDate().addDays(6));
    QVERIFY(paused > 0);
    QVERIFY(BillRepository::setActive(paused, false));

    BillListModel model;
    model.setShowPaused(true); // listed, but not counted
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.monthlyTotalFormatted(),
             CurrencyFormatter::format(Money::fromMinorUnits(25000)));
}

void TestBillListModel::dueLabel_describesTheDueDate()
{
    addBill(m_billsId, QStringLiteral("Today"), QStringLiteral("100"),
            QDate::currentDate());

    BillListModel model;
    const QModelIndex idx = model.index(0, 0);
    QVERIFY(!model.data(idx, BillListModel::DueLabelRole).toString().isEmpty());
    QCOMPARE(model.data(idx, BillListModel::DaysUntilDueRole).toInt(), 0);
    QVERIFY(!model.data(idx, BillListModel::NextDueFormattedRole).toString().isEmpty());
}

void TestBillListModel::recurrenceLabel_isPresentForEveryRow()
{
    addBill(m_billsId, QStringLiteral("Monthly"), QStringLiteral("100"),
            QDate::currentDate().addDays(5), Recurrence::Monthly);
    addBill(m_billsId, QStringLiteral("Quarterly"), QStringLiteral("100"),
            QDate::currentDate().addDays(6), Recurrence::Quarterly);
    addBill(m_billsId, QStringLiteral("Yearly"), QStringLiteral("100"),
            QDate::currentDate().addDays(7), Recurrence::Yearly);

    BillListModel model;
    QCOMPARE(model.rowCount(), 3);
    QStringList labels;
    for (int row = 0; row < model.rowCount(); ++row)
        labels << model.data(model.index(row, 0),
                             BillListModel::RecurrenceLabelRole)
                      .toString();
    for (const QString& label : labels)
        QVERIFY(!label.isEmpty());
    // Three cadences, three distinct labels.
    QCOMPARE(QSet<QString>(labels.cbegin(), labels.cend()).size(), 3);
}

QTEST_GUILESS_MAIN(TestBillListModel)
#include "tst_billlistmodel.moc"
