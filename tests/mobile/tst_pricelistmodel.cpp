#include "testutils.h"

#include "backend/pricelistmodel.h"
#include "storage/priceitemrepository.h"
#include "utils/currencyformatter.h"

#include <QColor>
#include <QSignalSpy>
#include <QtTest>

class TestPriceListModel : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();

    void emptyModel_hasNoRows();
    void roleNames_coverEveryRole();
    void data_exposesEveryRole();
    void data_outOfRangeIsEmpty();

    void uncategorized_reportsNoCategory();
    void categoryColor_isAColorForEveryRow();

    void trend_isFlatUntilThePriceMoves();
    void trend_reportsARise();
    void trend_reportsAFall();
    void changePercent_isZeroWhenThePreviousPriceWasZero();

    void searchText_filtersAndSignals();
    void counts_summarizeTheVisibleRows();
    void refresh_picksUpExternalChanges();
    void get_matchesTheRoleData();
    void priceItemIdAt_guardsItsRange();

private:
    static int add(const QString& name, qint64 priceMinor,
                   std::optional<int> categoryId = std::nullopt,
                   const QString& unit = QStringLiteral("kg"));
    static void setPrice(int id, qint64 priceMinor);

    int m_groceriesId = -1;
};

void TestPriceListModel::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestPriceListModel::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_groceriesId > 0);
}

int TestPriceListModel::add(const QString& name, qint64 priceMinor,
                            std::optional<int> categoryId, const QString& unit)
{
    PriceItem item;
    item.name = name;
    item.unit = unit;
    item.price = Money::fromMinorUnits(priceMinor);
    item.categoryId = categoryId;
    const Result<int> added = PriceItemRepository::add(item);
    return added ? *added : -1;
}

void TestPriceListModel::setPrice(int id, qint64 priceMinor)
{
    const Result<PriceItem> existing = PriceItemRepository::fetch(id);
    QVERIFY(existing.has_value());
    PriceItem updated = *existing;
    updated.price = Money::fromMinorUnits(priceMinor);
    VERIFY_OK(PriceItemRepository::update(updated));
}

void TestPriceListModel::emptyModel_hasNoRows()
{
    PriceListModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.count(), 0);
    QCOMPARE(model.risenCount(), 0);
    QCOMPARE(model.fallenCount(), 0);
}

void TestPriceListModel::roleNames_coverEveryRole()
{
    PriceListModel model;
    // Every delegate binds by role name, so a missing entry — or a typo in
    // one — is a runtime error in QML rather than a compile error here.
    // Compared whole so an extra role cannot slip in unnamed either.
    const QHash<int, QByteArray> expected = {
        { PriceListModel::PriceItemIdRole, "priceItemId" },
        { PriceListModel::NameRole, "name" },
        { PriceListModel::UnitRole, "unit" },
        { PriceListModel::PriceMinorRole, "priceMinor" },
        { PriceListModel::PriceFormattedRole, "priceFormatted" },
        { PriceListModel::CategoryIdRole, "categoryId" },
        { PriceListModel::CategoryNameRole, "categoryName" },
        { PriceListModel::CategoryColorRole, "categoryColor" },
        { PriceListModel::HasCategoryRole, "hasCategory" },
        { PriceListModel::UpdatedAtFormattedRole, "updatedAtFormatted" },
        { PriceListModel::HasPreviousPriceRole, "hasPreviousPrice" },
        { PriceListModel::PreviousPriceFormattedRole, "previousPriceFormatted" },
        { PriceListModel::PriceRoseRole, "priceRose" },
        { PriceListModel::PriceFellRole, "priceFell" },
        { PriceListModel::ChangePercentRole, "changePercent" },
    };
    QCOMPARE(model.roleNames(), expected);
}

void TestPriceListModel::data_exposesEveryRole()
{
    const int id = add(QStringLiteral("Milk"), 4550, m_groceriesId,
                       QStringLiteral("litre"));
    QVERIFY(id > 0);

    PriceListModel model;
    QCOMPARE(model.rowCount(), 1);
    const QModelIndex idx = model.index(0, 0);

    QCOMPARE(model.data(idx, PriceListModel::PriceItemIdRole).toInt(), id);
    QCOMPARE(model.data(idx, PriceListModel::NameRole).toString(),
             QStringLiteral("Milk"));
    QCOMPARE(model.data(idx, PriceListModel::UnitRole).toString(),
             QStringLiteral("litre"));
    QCOMPARE(model.data(idx, PriceListModel::PriceMinorRole).toLongLong(), 4550);
    QCOMPARE(model.data(idx, PriceListModel::PriceFormattedRole).toString(),
             CurrencyFormatter::format(Money::fromMinorUnits(4550)));
    QCOMPARE(model.data(idx, PriceListModel::CategoryIdRole).toInt(), m_groceriesId);
    QCOMPARE(model.data(idx, PriceListModel::CategoryNameRole).toString(),
             QStringLiteral("Groceries"));
    QVERIFY(model.data(idx, PriceListModel::HasCategoryRole).toBool());
    QVERIFY(!model.data(idx, PriceListModel::UpdatedAtFormattedRole).toString().isEmpty());
}

void TestPriceListModel::data_outOfRangeIsEmpty()
{
    QVERIFY(add(QStringLiteral("Milk"), 4550) > 0);

    PriceListModel model;
    QVERIFY(!model.data(model.index(5, 0), PriceListModel::NameRole).isValid());
    QVERIFY(!model.data(QModelIndex(), PriceListModel::NameRole).isValid());
    // An unknown role must be empty, not a stray value from another role.
    QVERIFY(!model.data(model.index(0, 0), Qt::UserRole + 999).isValid());
}

void TestPriceListModel::uncategorized_reportsNoCategory()
{
    QVERIFY(add(QStringLiteral("Batteries"), 12000) > 0);

    PriceListModel model;
    const QModelIndex idx = model.index(0, 0);
    QVERIFY(!model.data(idx, PriceListModel::HasCategoryRole).toBool());
    // -1, not 0: QML reads 0 as a real id in the category picker.
    QCOMPARE(model.data(idx, PriceListModel::CategoryIdRole).toInt(), -1);
    QVERIFY(model.data(idx, PriceListModel::CategoryNameRole).toString().isEmpty());
}

void TestPriceListModel::categoryColor_isAColorForEveryRow()
{
    QVERIFY(add(QStringLiteral("Milk"), 4550, m_groceriesId) > 0);
    QVERIFY(add(QStringLiteral("Batteries"), 12000) > 0);

    PriceListModel model;
    QCOMPARE(model.rowCount(), 2);
    for (int row = 0; row < model.rowCount(); ++row) {
        const QVariant color =
            model.data(model.index(row, 0), PriceListModel::CategoryColorRole);
        QVERIFY(color.canConvert<QColor>());
        QVERIFY(color.value<QColor>().isValid());
    }
}

void TestPriceListModel::trend_isFlatUntilThePriceMoves()
{
    QVERIFY(add(QStringLiteral("Milk"), 4550) > 0);

    PriceListModel model;
    const QModelIndex idx = model.index(0, 0);
    QVERIFY(!model.data(idx, PriceListModel::HasPreviousPriceRole).toBool());
    QVERIFY(!model.data(idx, PriceListModel::PriceRoseRole).toBool());
    QVERIFY(!model.data(idx, PriceListModel::PriceFellRole).toBool());
    QCOMPARE(model.data(idx, PriceListModel::ChangePercentRole).toDouble(), 0.0);
    QVERIFY(model.data(idx, PriceListModel::PreviousPriceFormattedRole)
                .toString()
                .isEmpty());
}

void TestPriceListModel::trend_reportsARise()
{
    const int id = add(QStringLiteral("Milk"), 4000);
    QVERIFY(id > 0);
    setPrice(id, 5000);

    PriceListModel model;
    const QModelIndex idx = model.index(0, 0);
    QVERIFY(model.data(idx, PriceListModel::HasPreviousPriceRole).toBool());
    QVERIFY(model.data(idx, PriceListModel::PriceRoseRole).toBool());
    QVERIFY(!model.data(idx, PriceListModel::PriceFellRole).toBool());
    QCOMPARE(model.data(idx, PriceListModel::ChangePercentRole).toDouble(), 25.0);
    QCOMPARE(model.data(idx, PriceListModel::PreviousPriceFormattedRole).toString(),
             CurrencyFormatter::format(Money::fromMinorUnits(4000)));
}

void TestPriceListModel::trend_reportsAFall()
{
    const int id = add(QStringLiteral("Rice"), 4000);
    QVERIFY(id > 0);
    setPrice(id, 3000);

    PriceListModel model;
    const QModelIndex idx = model.index(0, 0);
    QVERIFY(model.data(idx, PriceListModel::PriceFellRole).toBool());
    QVERIFY(!model.data(idx, PriceListModel::PriceRoseRole).toBool());
    QCOMPARE(model.data(idx, PriceListModel::ChangePercentRole).toDouble(), -25.0);
}

void TestPriceListModel::changePercent_isZeroWhenThePreviousPriceWasZero()
{
    // "Not priced yet" -> priced. There is no percentage against zero, and
    // the delegate must not be handed an infinity to render.
    const int id = add(QStringLiteral("Bread"), 0);
    QVERIFY(id > 0);
    setPrice(id, 1500);

    PriceListModel model;
    const QModelIndex idx = model.index(0, 0);
    QVERIFY(model.data(idx, PriceListModel::PriceRoseRole).toBool());
    QCOMPARE(model.data(idx, PriceListModel::ChangePercentRole).toDouble(), 0.0);
}

void TestPriceListModel::searchText_filtersAndSignals()
{
    QVERIFY(add(QStringLiteral("Milk"), 4550, std::nullopt,
                QStringLiteral("litre")) > 0);
    QVERIFY(add(QStringLiteral("Rice"), 3000, std::nullopt, QStringLiteral("kg")) > 0);

    PriceListModel model;
    QCOMPARE(model.rowCount(), 2);

    QSignalSpy searchSpy(&model, &PriceListModel::searchTextChanged);
    QSignalSpy countSpy(&model, &PriceListModel::countChanged);

    model.setSearchText(QStringLiteral("mil"));
    QCOMPARE(searchSpy.count(), 1);
    QCOMPARE(countSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);

    // Setting the same text again is not a change and must not reload.
    model.setSearchText(QStringLiteral("mil"));
    QCOMPARE(searchSpy.count(), 1);

    model.setSearchText(QString());
    QCOMPARE(model.rowCount(), 2);
}

void TestPriceListModel::counts_summarizeTheVisibleRows()
{
    const int up = add(QStringLiteral("Milk"), 4000);
    const int down = add(QStringLiteral("Rice"), 4000);
    QVERIFY(add(QStringLiteral("Salt"), 500) > 0); // never changed
    QVERIFY(up > 0);
    QVERIFY(down > 0);
    setPrice(up, 5000);
    setPrice(down, 3000);

    PriceListModel model;
    QCOMPARE(model.count(), 3);
    QCOMPARE(model.risenCount(), 1);
    QCOMPARE(model.fallenCount(), 1);

    // The counts describe what is on screen, so they follow the filter.
    model.setSearchText(QStringLiteral("Milk"));
    QCOMPARE(model.count(), 1);
    QCOMPARE(model.risenCount(), 1);
    QCOMPARE(model.fallenCount(), 0);
}

void TestPriceListModel::refresh_picksUpExternalChanges()
{
    PriceListModel model;
    QCOMPARE(model.rowCount(), 0);

    QVERIFY(add(QStringLiteral("Milk"), 4550) > 0);
    QCOMPARE(model.rowCount(), 0); // not until it is told

    QSignalSpy countSpy(&model, &PriceListModel::countChanged);
    model.refresh();
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(countSpy.count(), 1);
}

void TestPriceListModel::get_matchesTheRoleData()
{
    const int id = add(QStringLiteral("Milk"), 4550, m_groceriesId);
    QVERIFY(id > 0);

    PriceListModel model;
    const QVariantMap row = model.get(0);
    // The action sheets read rows through get(), so it has to carry the
    // same keys the delegates bind to.
    QCOMPARE(row.value(QStringLiteral("priceItemId")).toInt(), id);
    QCOMPARE(row.value(QStringLiteral("name")).toString(), QStringLiteral("Milk"));
    QCOMPARE(row.value(QStringLiteral("priceMinor")).toLongLong(), 4550);
    QVERIFY(row.value(QStringLiteral("hasCategory")).toBool());
    QCOMPARE(row.size(), model.roleNames().size());

    QVERIFY(model.get(-1).isEmpty());
    QVERIFY(model.get(9).isEmpty());
}

void TestPriceListModel::priceItemIdAt_guardsItsRange()
{
    const int id = add(QStringLiteral("Milk"), 4550);
    QVERIFY(id > 0);

    PriceListModel model;
    QCOMPARE(model.priceItemIdAt(0), id);
    QCOMPARE(model.priceItemIdAt(-1), -1);
    QCOMPARE(model.priceItemIdAt(7), -1);
}

QTEST_GUILESS_MAIN(TestPriceListModel)
#include "tst_pricelistmodel.moc"
