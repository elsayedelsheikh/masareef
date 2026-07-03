#include "testutils.h"

#include "backend/categorylistmodel.h"
#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"
#include "utils/palette.h"

#include <QSignalSpy>
#include <QtTest>

class TestCategoryListModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void listsSeededCategories();
    void roleNames_coverAllRoles();
    void rows_exposeCategoryRoles();
    void add_appendsCategory();
    void add_duplicateNameFailsWithError();
    void rename_updatesRow();
    void rename_emptyNameFailsWithError();
    void setColor_updatesRow();
    void remove_deletesUserCategory();
    void remove_systemCategoryFailsWithError();
    void remove_inUseCategoryFailsWithError();
    void inUse_reflectsExpenses();
    void suggestedColor_returnsFirstFreeSlot();
    void paletteColors_listsCategoricalSlots();
    void color_followsPaletteMode();

private:
    int rowOf(const CategoryListModel& model, const QString& name);
};

void TestCategoryListModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestCategoryListModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    Palette::setMode(Palette::Mode::Light);
}

int TestCategoryListModel::rowOf(const CategoryListModel& model, const QString& name)
{
    for (int row = 0; row < model.rowCount(); ++row) {
        if (model.data(model.index(row, 0), CategoryListModel::NameRole).toString()
            == name)
            return row;
    }
    return -1;
}

void TestCategoryListModel::listsSeededCategories()
{
    CategoryListModel model;
    QCOMPARE(model.rowCount(), int(CategoryRepository::all().size()));
    QVERIFY(rowOf(model, QStringLiteral("Bills")) >= 0);
    QVERIFY(rowOf(model, QStringLiteral("Groceries")) >= 0);
}

void TestCategoryListModel::roleNames_coverAllRoles()
{
    CategoryListModel model;
    const QHash<int, QByteArray> roles = model.roleNames();
    const QList<QByteArray> expected = {
        "categoryId", "name", "color", "isSystem", "inUse",
    };
    for (const QByteArray& role : expected)
        QVERIFY2(roles.values().contains(role), role.constData());
}

void TestCategoryListModel::rows_exposeCategoryRoles()
{
    CategoryListModel model;
    const int row = rowOf(model, QStringLiteral("Bills"));
    QVERIFY(row >= 0);

    const QModelIndex index = model.index(row, 0);
    QCOMPARE(model.data(index, CategoryListModel::CategoryIdRole).toInt(),
             TestUtils::categoryId(QStringLiteral("Bills")));
    QCOMPARE(model.data(index, CategoryListModel::IsSystemRole).toBool(), true);
    QCOMPARE(model.data(index, CategoryListModel::InUseRole).toBool(), false);
    QCOMPARE(model.data(index, CategoryListModel::ColorRole).value<QColor>(),
             QColor(QStringLiteral("#2a78d6")));
}

void TestCategoryListModel::add_appendsCategory()
{
    CategoryListModel model;
    const int before = model.rowCount();

    QVERIFY(model.add(QStringLiteral("Coffee"), QStringLiteral("#008300")));
    QCOMPARE(model.rowCount(), before + 1);

    const int row = rowOf(model, QStringLiteral("Coffee"));
    QVERIFY(row >= 0);
    QCOMPARE(model.data(model.index(row, 0), CategoryListModel::IsSystemRole).toBool(),
             false);
}

void TestCategoryListModel::add_duplicateNameFailsWithError()
{
    CategoryListModel model;
    const int before = model.rowCount();

    QVERIFY(!model.add(QStringLiteral("Bills"), QStringLiteral("#008300")));
    QVERIFY(!model.lastError().isEmpty());
    QCOMPARE(model.rowCount(), before);
}

void TestCategoryListModel::rename_updatesRow()
{
    CategoryListModel model;
    QVERIFY(model.add(QStringLiteral("Coffee"), QStringLiteral("#008300")));
    const int id = model.data(model.index(rowOf(model, QStringLiteral("Coffee")), 0),
                              CategoryListModel::CategoryIdRole)
                       .toInt();

    QVERIFY(model.rename(id, QStringLiteral("Tea")));
    QVERIFY(rowOf(model, QStringLiteral("Tea")) >= 0);
    QCOMPARE(rowOf(model, QStringLiteral("Coffee")), -1);
}

void TestCategoryListModel::rename_emptyNameFailsWithError()
{
    CategoryListModel model;
    const int id = TestUtils::categoryId(QStringLiteral("Bills"));

    QVERIFY(!model.rename(id, QStringLiteral("   ")));
    QVERIFY(!model.lastError().isEmpty());
}

void TestCategoryListModel::setColor_updatesRow()
{
    CategoryListModel model;
    QVERIFY(model.add(QStringLiteral("Coffee"), QStringLiteral("#008300")));
    const int row = rowOf(model, QStringLiteral("Coffee"));
    const int id = model.data(model.index(row, 0), CategoryListModel::CategoryIdRole)
                       .toInt();

    QSignalSpy changedSpy(&model, &QAbstractItemModel::dataChanged);
    QVERIFY(model.setColor(id, QStringLiteral("#4a3aa7")));
    QCOMPARE(model.data(model.index(rowOf(model, QStringLiteral("Coffee")), 0),
                        CategoryListModel::ColorRole)
                 .value<QColor>(),
             QColor(QStringLiteral("#4a3aa7")));
    QVERIFY(changedSpy.count() >= 1);
}

void TestCategoryListModel::remove_deletesUserCategory()
{
    CategoryListModel model;
    QVERIFY(model.add(QStringLiteral("Coffee"), QStringLiteral("#008300")));
    const int before = model.rowCount();
    const int id = model.data(model.index(rowOf(model, QStringLiteral("Coffee")), 0),
                              CategoryListModel::CategoryIdRole)
                       .toInt();

    QVERIFY(model.remove(id));
    QCOMPARE(model.rowCount(), before - 1);
    QCOMPARE(rowOf(model, QStringLiteral("Coffee")), -1);
}

void TestCategoryListModel::remove_systemCategoryFailsWithError()
{
    CategoryListModel model;
    const int before = model.rowCount();

    QVERIFY(!model.remove(TestUtils::categoryId(QStringLiteral("Bills"))));
    QVERIFY(!model.lastError().isEmpty());
    QCOMPARE(model.rowCount(), before);
}

void TestCategoryListModel::remove_inUseCategoryFailsWithError()
{
    CategoryListModel model;
    QVERIFY(model.add(QStringLiteral("Coffee"), QStringLiteral("#008300")));
    const int id = model.data(model.index(rowOf(model, QStringLiteral("Coffee")), 0),
                              CategoryListModel::CategoryIdRole)
                       .toInt();
    VERIFY_OK(ExpenseRepository::add({ .categoryId = id,
                                       .amount = Money::fromMinorUnits(500),
                                       .description = QStringLiteral("Latte"),
                                       .date = QDate(2026, 6, 1) }));
    model.refresh();

    QVERIFY(!model.remove(id));
    QVERIFY(!model.lastError().isEmpty());
    QVERIFY(rowOf(model, QStringLiteral("Coffee")) >= 0);
}

void TestCategoryListModel::inUse_reflectsExpenses()
{
    CategoryListModel model;
    const int billsRow = rowOf(model, QStringLiteral("Bills"));
    QCOMPARE(model.data(model.index(billsRow, 0), CategoryListModel::InUseRole).toBool(),
             false);

    VERIFY_OK(
        ExpenseRepository::add({ .categoryId = TestUtils::categoryId(QStringLiteral("Bills")),
                                 .amount = Money::fromMinorUnits(500),
                                 .description = QStringLiteral("Water"),
                                 .date = QDate(2026, 6, 1) }));
    model.refresh();

    QCOMPARE(model.data(model.index(rowOf(model, QStringLiteral("Bills")), 0),
                        CategoryListModel::InUseRole)
                 .toBool(),
             true);
}

void TestCategoryListModel::suggestedColor_returnsFirstFreeSlot()
{
    CategoryListModel model;
    // Seeds occupy slots 1-3, so the first free categorical slot is #008300.
    QCOMPARE(model.suggestedColor(), QString::fromLatin1(Palette::kCategorical[3]));
}

void TestCategoryListModel::paletteColors_listsCategoricalSlots()
{
    CategoryListModel model;
    const QStringList colors = model.paletteColors();
    QCOMPARE(colors.size(), Palette::kCategoricalCount);
    QCOMPARE(colors.first(), QString::fromLatin1(Palette::kCategorical[0]));
}

void TestCategoryListModel::color_followsPaletteMode()
{
    CategoryListModel model;
    const QModelIndex index = model.index(rowOf(model, QStringLiteral("Bills")), 0);

    Palette::setMode(Palette::Mode::Light);
    QCOMPARE(model.data(index, CategoryListModel::ColorRole).value<QColor>(),
             QColor(QStringLiteral("#2a78d6")));

    Palette::setMode(Palette::Mode::Dark);
    QCOMPARE(model.data(index, CategoryListModel::ColorRole).value<QColor>(),
             QColor(QStringLiteral("#3987e5")));
}

QTEST_GUILESS_MAIN(TestCategoryListModel)
#include "tst_categorylistmodel.moc"
