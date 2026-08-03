#include "testutils.h"

#include "storage/priceitemrepository.h"

#include <QtTest>

class TestPriceItemRepository : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();

    void add_storesItemAndSeedsHistory();
    void add_rejectsEmptyName();
    void add_rejectsNegativePrice();
    void add_acceptsZeroPrice();
    void add_rejectsDuplicateNameIgnoringCase();
    void add_leavesNoHistoryWhenItFails();
    void add_allowsMissingCategory();

    void all_ordersByNameCaseInsensitively();
    void all_filtersByNameOrUnit();
    void all_reportsNoPreviousPriceForANewItem();

    void update_recordsHistoryOnlyForAPriceChange();
    void update_exposesThePreviousPrice();
    void update_reportsTheDirectionOfTheChange();
    void update_rejectsAnUnknownId();
    void update_rejectsANameTakenByAnotherItem();
    void update_rejectsAnEmptyNameOrNegativePrice();
    void update_keepsTheDateWhenOnlyTheNameChanges();

    void remove_takesHistoryWithIt();
    void remove_reportsAnUnknownId();

    void deletingACategory_keepsTheItem();

private:
    static int add(const QString& name, const QString& unit, qint64 priceMinor,
                   std::optional<int> categoryId = std::nullopt);
    static void setPrice(int id, qint64 priceMinor);

    int m_groceriesId = -1;
};

void TestPriceItemRepository::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestPriceItemRepository::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_groceriesId > 0);
}

int TestPriceItemRepository::add(const QString& name, const QString& unit,
                                 qint64 priceMinor, std::optional<int> categoryId)
{
    PriceItem item;
    item.name = name;
    item.unit = unit;
    item.price = Money::fromMinorUnits(priceMinor);
    item.categoryId = categoryId;
    const Result<int> added = PriceItemRepository::add(item);
    return added ? *added : -1;
}

void TestPriceItemRepository::setPrice(int id, qint64 priceMinor)
{
    const Result<PriceItem> existing = PriceItemRepository::fetch(id);
    QVERIFY(existing.has_value());
    PriceItem updated = *existing;
    updated.price = Money::fromMinorUnits(priceMinor);
    VERIFY_OK(PriceItemRepository::update(updated));
}

void TestPriceItemRepository::add_storesItemAndSeedsHistory()
{
    const int id = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4550,
                       m_groceriesId);
    QVERIFY(id > 0);

    const Result<PriceItem> fetched = PriceItemRepository::fetch(id);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->name, QStringLiteral("Milk"));
    QCOMPARE(fetched->unit, QStringLiteral("litre"));
    QCOMPARE(fetched->price.minorUnits(), 4550);
    QCOMPARE(fetched->categoryId.value_or(-1), m_groceriesId);
    QCOMPARE(fetched->categoryName, QStringLiteral("Groceries"));
    QCOMPARE(fetched->updatedAt, QDate::currentDate());

    // The first price is history too, so a later change has something to
    // compare against.
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 1);
}

void TestPriceItemRepository::add_rejectsEmptyName()
{
    QCOMPARE(add(QStringLiteral("   "), QStringLiteral("kg"), 100), -1);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
}

void TestPriceItemRepository::add_rejectsNegativePrice()
{
    QCOMPARE(add(QStringLiteral("Milk"), QStringLiteral("litre"), -1), -1);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
}

void TestPriceItemRepository::add_acceptsZeroPrice()
{
    // "On the list, not priced yet" is a state worth being able to save.
    QVERIFY(add(QStringLiteral("Bread"), QStringLiteral("piece"), 0) > 0);
}

void TestPriceItemRepository::add_rejectsDuplicateNameIgnoringCase()
{
    QVERIFY(add(QStringLiteral("Milk"), QStringLiteral("litre"), 4550) > 0);
    QCOMPARE(add(QStringLiteral("milk"), QStringLiteral("litre"), 5000), -1);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 1);
}

void TestPriceItemRepository::add_leavesNoHistoryWhenItFails()
{
    QVERIFY(add(QStringLiteral("Milk"), QStringLiteral("litre"), 4550) > 0);
    QCOMPARE(add(QStringLiteral("Milk"), QStringLiteral("litre"), 5000), -1);
    // The rejected insert must not leave a history row behind it.
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 1);
}

void TestPriceItemRepository::add_allowsMissingCategory()
{
    const int id = add(QStringLiteral("Batteries"), QStringLiteral("pack"), 12000);
    QVERIFY(id > 0);

    const Result<PriceItem> fetched = PriceItemRepository::fetch(id);
    QVERIFY(fetched.has_value());
    QVERIFY(!fetched->categoryId.has_value());
    QVERIFY(fetched->categoryName.isEmpty());
}

void TestPriceItemRepository::all_ordersByNameCaseInsensitively()
{
    QVERIFY(add(QStringLiteral("banana"), QStringLiteral("kg"), 100) > 0);
    QVERIFY(add(QStringLiteral("Apple"), QStringLiteral("kg"), 200) > 0);
    QVERIFY(add(QStringLiteral("cherry"), QStringLiteral("kg"), 300) > 0);

    const QList<PriceItem> items = PriceItemRepository::all();
    QCOMPARE(items.size(), 3);
    QCOMPARE(items.at(0).name, QStringLiteral("Apple"));
    QCOMPARE(items.at(1).name, QStringLiteral("banana"));
    QCOMPARE(items.at(2).name, QStringLiteral("cherry"));
}

void TestPriceItemRepository::all_filtersByNameOrUnit()
{
    QVERIFY(add(QStringLiteral("Milk"), QStringLiteral("litre"), 4550) > 0);
    QVERIFY(add(QStringLiteral("Rice"), QStringLiteral("kg"), 3000) > 0);

    QCOMPARE(PriceItemRepository::all(QStringLiteral("mil")).size(), 1);
    QCOMPARE(PriceItemRepository::all(QStringLiteral("kg")).size(), 1);
    QCOMPARE(PriceItemRepository::all(QStringLiteral("  ")).size(), 2);
    QCOMPARE(PriceItemRepository::all(QStringLiteral("zzz")).size(), 0);
}

void TestPriceItemRepository::all_reportsNoPreviousPriceForANewItem()
{
    QVERIFY(add(QStringLiteral("Milk"), QStringLiteral("litre"), 4550) > 0);

    const QList<PriceItem> items = PriceItemRepository::all();
    QCOMPARE(items.size(), 1);
    QVERIFY(!items.at(0).previousPrice.has_value());
    QVERIFY(!items.at(0).priceRose());
    QVERIFY(!items.at(0).priceFell());
}

void TestPriceItemRepository::update_recordsHistoryOnlyForAPriceChange()
{
    const int id = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4550);
    QVERIFY(id > 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 1);

    // A rename is not a price movement.
    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    PriceItem renamed = *stored;
    renamed.name = QStringLiteral("Fresh milk");
    VERIFY_OK(PriceItemRepository::update(renamed));
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 1);

    setPrice(id, 5000);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 2);
}

void TestPriceItemRepository::update_exposesThePreviousPrice()
{
    const int id = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4000);
    QVERIFY(id > 0);
    setPrice(id, 5000);

    const Result<PriceItem> fetched = PriceItemRepository::fetch(id);
    QVERIFY(fetched.has_value());
    QCOMPARE(fetched->price.minorUnits(), 5000);
    QVERIFY(fetched->previousPrice.has_value());
    QCOMPARE(fetched->previousPrice->minorUnits(), 4000);

    // A third change reports the second price, not the first.
    setPrice(id, 5500);
    const Result<PriceItem> again = PriceItemRepository::fetch(id);
    QVERIFY(again.has_value());
    QVERIFY(again->previousPrice.has_value());
    QCOMPARE(again->previousPrice->minorUnits(), 5000);
}

void TestPriceItemRepository::update_reportsTheDirectionOfTheChange()
{
    const int up = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4000);
    const int down = add(QStringLiteral("Rice"), QStringLiteral("kg"), 4000);
    QVERIFY(up > 0);
    QVERIFY(down > 0);

    setPrice(up, 5000);
    setPrice(down, 3000);

    const Result<PriceItem> rose = PriceItemRepository::fetch(up);
    QVERIFY(rose.has_value());
    QVERIFY(rose->priceRose());
    QVERIFY(!rose->priceFell());

    const Result<PriceItem> fell = PriceItemRepository::fetch(down);
    QVERIFY(fell.has_value());
    QVERIFY(fell->priceFell());
    QVERIFY(!fell->priceRose());
}

void TestPriceItemRepository::update_rejectsAnUnknownId()
{
    PriceItem missing;
    missing.id = 4242;
    missing.name = QStringLiteral("Ghost");
    missing.price = Money::fromMinorUnits(100);
    QVERIFY(!PriceItemRepository::update(missing));
}

void TestPriceItemRepository::update_rejectsANameTakenByAnotherItem()
{
    const int milk = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4000);
    const int rice = add(QStringLiteral("Rice"), QStringLiteral("kg"), 3000);
    QVERIFY(milk > 0);
    QVERIFY(rice > 0);

    // The name index is case-insensitive, so "milk" collides with "Milk".
    const Result<PriceItem> stored = PriceItemRepository::fetch(rice);
    QVERIFY(stored.has_value());
    PriceItem renamed = *stored;
    renamed.name = QStringLiteral("milk");
    QVERIFY(!PriceItemRepository::update(renamed));

    // The rejected rename must leave the row exactly as it was.
    const Result<PriceItem> unchanged = PriceItemRepository::fetch(rice);
    QVERIFY(unchanged.has_value());
    QCOMPARE(unchanged->name, QStringLiteral("Rice"));
    QCOMPARE(unchanged->price.minorUnits(), 3000);
}

void TestPriceItemRepository::update_rejectsAnEmptyNameOrNegativePrice()
{
    const int id = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4000);
    QVERIFY(id > 0);

    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());

    PriceItem blank = *stored;
    blank.name = QStringLiteral("   ");
    QVERIFY(!PriceItemRepository::update(blank));

    PriceItem negative = *stored;
    negative.price = Money::fromMinorUnits(-1);
    QVERIFY(!PriceItemRepository::update(negative));

    const Result<PriceItem> unchanged = PriceItemRepository::fetch(id);
    QVERIFY(unchanged.has_value());
    QCOMPARE(unchanged->name, QStringLiteral("Milk"));
    QCOMPARE(unchanged->price.minorUnits(), 4000);
    // Neither rejection may have appended to the history.
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 1);
}

// updated_at answers "when was this price last checked", so an edit that
// does not touch the price must not move it forward.
void TestPriceItemRepository::update_keepsTheDateWhenOnlyTheNameChanges()
{
    const QDate lastChecked = QDate::currentDate().addDays(-30);
    PriceItem item;
    item.name = QStringLiteral("Milk");
    item.unit = QStringLiteral("litre");
    item.price = Money::fromMinorUnits(4000);
    item.updatedAt = lastChecked;
    const Result<int> added = PriceItemRepository::add(item);
    QVERIFY(added.has_value());

    const Result<PriceItem> stored = PriceItemRepository::fetch(*added);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->updatedAt, lastChecked);

    PriceItem renamed = *stored;
    renamed.name = QStringLiteral("Fresh milk");
    renamed.unit = QStringLiteral("carton");
    VERIFY_OK(PriceItemRepository::update(renamed));

    const Result<PriceItem> afterRename = PriceItemRepository::fetch(*added);
    QVERIFY(afterRename.has_value());
    QCOMPARE(afterRename->updatedAt, lastChecked);

    // A genuine price change does move it.
    PriceItem repriced = *afterRename;
    repriced.price = Money::fromMinorUnits(5000);
    VERIFY_OK(PriceItemRepository::update(repriced));

    const Result<PriceItem> afterReprice = PriceItemRepository::fetch(*added);
    QVERIFY(afterReprice.has_value());
    QCOMPARE(afterReprice->updatedAt, QDate::currentDate());
}

void TestPriceItemRepository::remove_takesHistoryWithIt()
{
    const int id = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4000);
    QVERIFY(id > 0);
    setPrice(id, 5000);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 2);

    VERIFY_OK(PriceItemRepository::remove(id));
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 0);
}

void TestPriceItemRepository::remove_reportsAnUnknownId()
{
    QVERIFY(!PriceItemRepository::remove(4242));
}

void TestPriceItemRepository::deletingACategory_keepsTheItem()
{
    // Unlike an expense, a price book entry outlives its category: the
    // reference is nulled rather than the delete being refused.
    const int id = add(QStringLiteral("Milk"), QStringLiteral("litre"), 4000,
                       m_groceriesId);
    QVERIFY(id > 0);

    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM categories WHERE id = ?"));
    query.addBindValue(m_groceriesId);
    QVERIFY(query.exec());

    const Result<PriceItem> fetched = PriceItemRepository::fetch(id);
    QVERIFY(fetched.has_value());
    QVERIFY(!fetched->categoryId.has_value());
}

QTEST_GUILESS_MAIN(TestPriceItemRepository)
#include "tst_priceitemrepository.moc"
