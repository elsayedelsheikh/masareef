#include "testutils.h"

#include "backend/expenselistmodel.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

#include <QSignalSpy>
#include <QtTest>

class TestExpenseListModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void emptyModel_hasNoRowsAndZeroTotal();
    void roleNames_coverAllRoles();
    void rows_exposeExpenseRoles();
    void rows_sortedNewestFirst();
    void categoryFilter_narrowsRows();
    void dateFilter_narrowsRows();
    void searchText_matchesDescriptionAndNotes();
    void totalFormatted_tracksFilter();
    void removeAt_deletesRowAndUpdatesTotal();
    void removeAt_invalidRowFails();
    void expenseIdAt_returnsIdOrMinusOne();
    void refresh_picksUpExternalChanges();
    void categoryColor_followsPaletteMode();
    void get_returnsRoleMap();
    void dateSection_isIsoDate();

private:
    int addExpense(int categoryId, qint64 minor, const QString& description,
                   QDate date, const QString& notes = {});

    int m_billsId = -1;
    int m_groceriesId = -1;
};

void TestExpenseListModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestExpenseListModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    Palette::setMode(Palette::Mode::Light);
    m_billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_billsId > 0);
    QVERIFY(m_groceriesId > 0);
}

int TestExpenseListModel::addExpense(int categoryId, qint64 minor,
                                     const QString& description, QDate date,
                                     const QString& notes)
{
    const Result<int> added =
        ExpenseRepository::add({ .categoryId = categoryId,
                                 .amount = Money::fromMinorUnits(minor),
                                 .description = description,
                                 .date = date,
                                 .notes = notes });
    if (!added) {
        qWarning("addExpense failed: %s", qPrintable(added.error().message));
        return -1;
    }
    return *added;
}

void TestExpenseListModel::emptyModel_hasNoRowsAndZeroTotal()
{
    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.count(), 0);
    QCOMPARE(model.totalFormatted(), CurrencyFormatter::format(Money{}));
}

void TestExpenseListModel::roleNames_coverAllRoles()
{
    ExpenseListModel model;
    const QHash<int, QByteArray> roles = model.roleNames();
    const QList<QByteArray> expected = {
        "expenseId", "date", "categoryId", "categoryName", "categoryColor",
        "description", "amountMinor", "amountFormatted", "notes",
    };
    for (const QByteArray& role : expected)
        QVERIFY2(roles.values().contains(role), role.constData());
}

void TestExpenseListModel::rows_exposeExpenseRoles()
{
    const int id = addExpense(m_billsId, 25099, QStringLiteral("Electricity"),
                              QDate(2026, 6, 15), QStringLiteral("June meter"));
    QVERIFY(id > 0);

    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 1);

    const QModelIndex index = model.index(0, 0);
    QCOMPARE(model.data(index, ExpenseListModel::ExpenseIdRole).toInt(), id);
    QCOMPARE(model.data(index, ExpenseListModel::DateRole).toDate(), QDate(2026, 6, 15));
    QCOMPARE(model.data(index, ExpenseListModel::CategoryIdRole).toInt(), m_billsId);
    QCOMPARE(model.data(index, ExpenseListModel::CategoryNameRole).toString(),
             QStringLiteral("Bills"));
    QCOMPARE(model.data(index, ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Electricity"));
    QCOMPARE(model.data(index, ExpenseListModel::AmountMinorRole).toLongLong(),
             qint64(25099));
    QCOMPARE(model.data(index, ExpenseListModel::AmountFormattedRole).toString(),
             CurrencyFormatter::format(Money::fromMinorUnits(25099)));
    QCOMPARE(model.data(index, ExpenseListModel::NotesRole).toString(),
             QStringLiteral("June meter"));
}

void TestExpenseListModel::rows_sortedNewestFirst()
{
    addExpense(m_billsId, 1000, QStringLiteral("Older"), QDate(2026, 6, 1));
    addExpense(m_billsId, 2000, QStringLiteral("Newer"), QDate(2026, 6, 20));
    addExpense(m_billsId, 3000, QStringLiteral("Middle"), QDate(2026, 6, 10));

    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Newer"));
    QCOMPARE(model.data(model.index(1, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Middle"));
    QCOMPARE(model.data(model.index(2, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Older"));
}

void TestExpenseListModel::categoryFilter_narrowsRows()
{
    addExpense(m_billsId, 1000, QStringLiteral("Water"), QDate(2026, 6, 1));
    addExpense(m_groceriesId, 2000, QStringLiteral("Bread"), QDate(2026, 6, 2));

    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 2);

    QSignalSpy countSpy(&model, &ExpenseListModel::countChanged);
    model.setCategoryId(m_groceriesId);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Bread"));
    QVERIFY(countSpy.count() >= 1);

    model.setCategoryId(-1);
    QCOMPARE(model.rowCount(), 2);
}

void TestExpenseListModel::dateFilter_narrowsRows()
{
    addExpense(m_billsId, 1000, QStringLiteral("May"), QDate(2026, 5, 20));
    addExpense(m_billsId, 2000, QStringLiteral("June"), QDate(2026, 6, 10));

    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 2);

    model.setFromDate(QDate(2026, 6, 1));
    model.setToDate(QDate(2026, 6, 30));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("June"));

    model.setFromDate(QDate());
    model.setToDate(QDate());
    QCOMPARE(model.rowCount(), 2);
}

void TestExpenseListModel::searchText_matchesDescriptionAndNotes()
{
    addExpense(m_billsId, 1000, QStringLiteral("Electricity"), QDate(2026, 6, 1));
    addExpense(m_billsId, 2000, QStringLiteral("Internet"), QDate(2026, 6, 2),
               QStringLiteral("fiber plan"));

    ExpenseListModel model;
    model.setSearchText(QStringLiteral("elect"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Electricity"));

    model.setSearchText(QStringLiteral("fiber"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Internet"));

    model.setSearchText(QString());
    QCOMPARE(model.rowCount(), 2);
}

void TestExpenseListModel::totalFormatted_tracksFilter()
{
    addExpense(m_billsId, 10000, QStringLiteral("Water"), QDate(2026, 6, 1));
    addExpense(m_groceriesId, 5050, QStringLiteral("Bread"), QDate(2026, 6, 2));

    ExpenseListModel model;
    QCOMPARE(model.totalFormatted(),
             CurrencyFormatter::format(Money::fromMinorUnits(15050)));

    QSignalSpy totalSpy(&model, &ExpenseListModel::totalChanged);
    model.setCategoryId(m_billsId);
    QCOMPARE(model.totalFormatted(),
             CurrencyFormatter::format(Money::fromMinorUnits(10000)));
    QVERIFY(totalSpy.count() >= 1);
}

void TestExpenseListModel::removeAt_deletesRowAndUpdatesTotal()
{
    addExpense(m_billsId, 1000, QStringLiteral("Keep"), QDate(2026, 6, 1));
    addExpense(m_billsId, 2000, QStringLiteral("Drop"), QDate(2026, 6, 2));

    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 2);

    QSignalSpy removedSpy(&model, &QAbstractItemModel::rowsRemoved);
    QVERIFY(model.removeAt(0)); // newest first -> "Drop"
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DescriptionRole).toString(),
             QStringLiteral("Keep"));
    QCOMPARE(model.totalFormatted(),
             CurrencyFormatter::format(Money::fromMinorUnits(1000)));
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);
}

void TestExpenseListModel::removeAt_invalidRowFails()
{
    ExpenseListModel model;
    QVERIFY(!model.removeAt(0));
    QVERIFY(!model.removeAt(-1));
}

void TestExpenseListModel::expenseIdAt_returnsIdOrMinusOne()
{
    const int id = addExpense(m_billsId, 1000, QStringLiteral("Water"), QDate(2026, 6, 1));

    ExpenseListModel model;
    QCOMPARE(model.expenseIdAt(0), id);
    QCOMPARE(model.expenseIdAt(1), -1);
    QCOMPARE(model.expenseIdAt(-1), -1);
}

void TestExpenseListModel::refresh_picksUpExternalChanges()
{
    ExpenseListModel model;
    QCOMPARE(model.rowCount(), 0);

    addExpense(m_billsId, 1000, QStringLiteral("Water"), QDate(2026, 6, 1));
    QCOMPARE(model.rowCount(), 0); // not yet visible

    model.refresh();
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.count(), 1);
}

void TestExpenseListModel::categoryColor_followsPaletteMode()
{
    // Bills is seeded with categorical slot 1 (#2a78d6); its dark-mode
    // counterpart is #3987e5 (see palette.cpp).
    addExpense(m_billsId, 1000, QStringLiteral("Water"), QDate(2026, 6, 1));

    ExpenseListModel model;
    const QModelIndex index = model.index(0, 0);

    Palette::setMode(Palette::Mode::Light);
    QCOMPARE(model.data(index, ExpenseListModel::CategoryColorRole).value<QColor>(),
             QColor(QStringLiteral("#2a78d6")));

    Palette::setMode(Palette::Mode::Dark);
    QCOMPARE(model.data(index, ExpenseListModel::CategoryColorRole).value<QColor>(),
             QColor(QStringLiteral("#3987e5")));
}

void TestExpenseListModel::get_returnsRoleMap()
{
    const int id = addExpense(m_billsId, 25099, QStringLiteral("Electricity"),
                              QDate(2026, 6, 15), QStringLiteral("June meter"));
    QVERIFY(id > 0);

    ExpenseListModel model;
    const QVariantMap row = model.get(0);
    QCOMPARE(row.value(QStringLiteral("expenseId")).toInt(), id);
    QCOMPARE(row.value(QStringLiteral("categoryId")).toInt(), m_billsId);
    QCOMPARE(row.value(QStringLiteral("amountMinor")).toLongLong(), qint64(25099));
    QCOMPARE(row.value(QStringLiteral("description")).toString(),
             QStringLiteral("Electricity"));
    QCOMPARE(row.value(QStringLiteral("date")).toDate(), QDate(2026, 6, 15));
    QCOMPARE(row.value(QStringLiteral("notes")).toString(),
             QStringLiteral("June meter"));

    QVERIFY(model.get(5).isEmpty());
    QVERIFY(model.get(-1).isEmpty());
}

void TestExpenseListModel::dateSection_isIsoDate()
{
    addExpense(m_billsId, 1000, QStringLiteral("Water"), QDate(2026, 6, 15));

    ExpenseListModel model;
    QCOMPARE(model.data(model.index(0, 0), ExpenseListModel::DateSectionRole).toString(),
             QStringLiteral("2026-06-15"));
}

QTEST_GUILESS_MAIN(TestExpenseListModel)
#include "tst_expenselistmodel.moc"
