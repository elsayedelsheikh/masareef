#include "testutils.h"

#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"

#include <QtTest>

class TestCategoryRepository : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void add_appearsInList();
    void add_duplicateNameFails();
    void rename_works();
    void setColor_works();
    void remove_userUnused();
    void remove_systemRefused();
    void remove_inUseRefused();
    void suggestedColor_avoidsUsedSlots();
};

void TestCategoryRepository::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestCategoryRepository::init()
{
    QVERIFY(TestUtils::resetDatabase());
}

static const Category* findByName(const QList<Category>& list, const QString& name)
{
    for (const Category& c : list) {
        if (c.name == name)
            return &c;
    }
    return nullptr;
}

void TestCategoryRepository::add_appearsInList()
{
    VERIFY_OK(CategoryRepository::add(QStringLiteral("Transport"),
                                      QStringLiteral("#008300")));
    const QList<Category> all = CategoryRepository::all();
    QCOMPARE(all.size(), 4);
    const Category* cat = findByName(all, QStringLiteral("Transport"));
    QVERIFY(cat);
    QCOMPARE(cat->type, CategoryType::User);
    QCOMPARE(cat->color, QStringLiteral("#008300"));
    QVERIFY(!cat->isSystem());
}

void TestCategoryRepository::add_duplicateNameFails()
{
    const Result<void> added =
        CategoryRepository::add(QStringLiteral("Bills"), QStringLiteral("#008300"));
    QVERIFY(!added);
    QVERIFY(!added.error().message.isEmpty());
    QCOMPARE(CategoryRepository::all().size(), 3);
}

void TestCategoryRepository::rename_works()
{
    VERIFY_OK(CategoryRepository::add(QStringLiteral("Transport"),
                                      QStringLiteral("#008300")));
    const int id = TestUtils::categoryId(QStringLiteral("Transport"));
    QVERIFY(id > 0);
    VERIFY_OK(CategoryRepository::rename(id, QStringLiteral("Commute")));
    QVERIFY(findByName(CategoryRepository::all(), QStringLiteral("Commute")));
    // Renaming onto an existing name must fail
    QVERIFY(!CategoryRepository::rename(id, QStringLiteral("Bills")));
}

void TestCategoryRepository::setColor_works()
{
    const int id = TestUtils::categoryId(QStringLiteral("Bills"));
    VERIFY_OK(CategoryRepository::setColor(id, QStringLiteral("#4a3aa7")));
    const QList<Category> all = CategoryRepository::all();
    const Category* cat = findByName(all, QStringLiteral("Bills"));
    QVERIFY(cat);
    QCOMPARE(cat->color, QStringLiteral("#4a3aa7"));
}

void TestCategoryRepository::remove_userUnused()
{
    VERIFY_OK(CategoryRepository::add(QStringLiteral("Transport"),
                                      QStringLiteral("#008300")));
    const int id = TestUtils::categoryId(QStringLiteral("Transport"));
    VERIFY_OK(CategoryRepository::remove(id));
    QCOMPARE(CategoryRepository::all().size(), 3);
}

void TestCategoryRepository::remove_systemRefused()
{
    const int id = TestUtils::categoryId(QStringLiteral("Bills"));
    const Result<void> removed = CategoryRepository::remove(id);
    QVERIFY(!removed);
    QVERIFY(!removed.error().message.isEmpty());
    QCOMPARE(CategoryRepository::all().size(), 3);
}

void TestCategoryRepository::remove_inUseRefused()
{
    VERIFY_OK(CategoryRepository::add(QStringLiteral("Transport"),
                                      QStringLiteral("#008300")));
    const int id = TestUtils::categoryId(QStringLiteral("Transport"));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = id,
                                       .amount = Money::fromMinorUnits(1500),
                                       .description = QStringLiteral("Bus"),
                                       .date = QDate(2026, 7, 1) }));
    QVERIFY(CategoryRepository::isInUse(id));
    const Result<void> removed = CategoryRepository::remove(id);
    QVERIFY(!removed);
    QVERIFY(!removed.error().message.isEmpty());
    QCOMPARE(CategoryRepository::all().size(), 4);
}

void TestCategoryRepository::suggestedColor_avoidsUsedSlots()
{
    // Seeds use slots 1-3, so the suggestion must be a different palette slot
    const QString suggestion = CategoryRepository::suggestedColor();
    QVERIFY(!suggestion.isEmpty());
    QVERIFY(suggestion != QStringLiteral("#2a78d6"));
    QVERIFY(suggestion != QStringLiteral("#1baf7a"));
    QVERIFY(suggestion != QStringLiteral("#eda100"));
}

QTEST_GUILESS_MAIN(TestCategoryRepository)
#include "tst_categoryrepository.moc"
