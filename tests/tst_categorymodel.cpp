#include "testutils.h"

#include "models/categorymodel.h"
#include "models/expensemodel.h"

#include <QtTest>

class TestCategoryModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void addCategory_appearsInList();
    void addCategory_duplicateNameFails();
    void renameCategory_works();
    void setColor_works();
    void removeCategory_userUnused();
    void removeCategory_systemRefused();
    void removeCategory_inUseRefused();
    void suggestedColor_avoidsUsedSlots();
};

void TestCategoryModel::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestCategoryModel::init()
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

void TestCategoryModel::addCategory_appearsInList()
{
    QString error;
    QVERIFY2(CategoryModel::addCategory(QStringLiteral("Transport"),
                                        QStringLiteral("#008300"), &error),
             qPrintable(error));
    const QList<Category> all = CategoryModel::allCategories();
    QCOMPARE(all.size(), 4);
    const Category* cat = findByName(all, QStringLiteral("Transport"));
    QVERIFY(cat);
    QCOMPARE(cat->type, QStringLiteral("user"));
    QCOMPARE(cat->color, QStringLiteral("#008300"));
    QVERIFY(!cat->isSystem());
}

void TestCategoryModel::addCategory_duplicateNameFails()
{
    QString error;
    QVERIFY(!CategoryModel::addCategory(QStringLiteral("Bills"),
                                        QStringLiteral("#008300"), &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(CategoryModel::allCategories().size(), 3);
}

void TestCategoryModel::renameCategory_works()
{
    QString error;
    QVERIFY(CategoryModel::addCategory(QStringLiteral("Transport"),
                                       QStringLiteral("#008300"), &error));
    const int id = TestUtils::categoryId(QStringLiteral("Transport"));
    QVERIFY(id > 0);
    QVERIFY2(CategoryModel::renameCategory(id, QStringLiteral("Commute"), &error),
             qPrintable(error));
    QVERIFY(findByName(CategoryModel::allCategories(), QStringLiteral("Commute")));
    // Renaming onto an existing name must fail
    QVERIFY(!CategoryModel::renameCategory(id, QStringLiteral("Bills"), &error));
}

void TestCategoryModel::setColor_works()
{
    const int id = TestUtils::categoryId(QStringLiteral("Bills"));
    QString error;
    QVERIFY2(CategoryModel::setColor(id, QStringLiteral("#4a3aa7"), &error), qPrintable(error));
    const Category* cat = nullptr;
    const QList<Category> all = CategoryModel::allCategories();
    cat = findByName(all, QStringLiteral("Bills"));
    QVERIFY(cat);
    QCOMPARE(cat->color, QStringLiteral("#4a3aa7"));
}

void TestCategoryModel::removeCategory_userUnused()
{
    QString error;
    QVERIFY(CategoryModel::addCategory(QStringLiteral("Transport"),
                                       QStringLiteral("#008300"), &error));
    const int id = TestUtils::categoryId(QStringLiteral("Transport"));
    QVERIFY2(CategoryModel::removeCategory(id, &error), qPrintable(error));
    QCOMPARE(CategoryModel::allCategories().size(), 3);
}

void TestCategoryModel::removeCategory_systemRefused()
{
    const int id = TestUtils::categoryId(QStringLiteral("Bills"));
    QString error;
    QVERIFY(!CategoryModel::removeCategory(id, &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(CategoryModel::allCategories().size(), 3);
}

void TestCategoryModel::removeCategory_inUseRefused()
{
    QString error;
    QVERIFY(CategoryModel::addCategory(QStringLiteral("Transport"),
                                       QStringLiteral("#008300"), &error));
    const int id = TestUtils::categoryId(QStringLiteral("Transport"));
    QVERIFY2(ExpenseModel::addExpense(id, 1500, QStringLiteral("Bus"),
                                      QDate(2026, 7, 1), QString(), QVariant(), &error),
             qPrintable(error));
    QVERIFY(CategoryModel::isInUse(id));
    QVERIFY(!CategoryModel::removeCategory(id, &error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(CategoryModel::allCategories().size(), 4);
}

void TestCategoryModel::suggestedColor_avoidsUsedSlots()
{
    // Seeds use slots 1-3, so the suggestion must be a different palette slot
    const QString suggestion = CategoryModel::suggestedColor();
    QVERIFY(!suggestion.isEmpty());
    QVERIFY(suggestion != QStringLiteral("#2a78d6"));
    QVERIFY(suggestion != QStringLiteral("#1baf7a"));
    QVERIFY(suggestion != QStringLiteral("#eda100"));
}

QTEST_GUILESS_MAIN(TestCategoryModel)
#include "tst_categorymodel.moc"
