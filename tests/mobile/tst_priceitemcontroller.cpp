#include "testutils.h"

#include "backend/priceitemcontroller.h"
#include "storage/priceitemrepository.h"

#include <QSignalSpy>
#include <QtTest>

// The controller is the boundary QML calls across: prices arrive as typed
// text and every failure has to come back as 0/false with a sentence the
// user can act on, never as a crash or a silent no-op.
class TestPriceItemController : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();

    void add_storesAndSignals();
    void add_acceptsArabicNumerals();
    void add_acceptsZero();
    void add_trimsWhitespace();
    void add_treatsNonPositiveCategoryAsUncategorized();
    void add_rejectsEmptyName();
    void add_rejectsUnparseablePrice();
    void add_rejectsDuplicateName();

    void update_changesTheRowAndSignals();
    void update_rejectsAnUnknownId();

    void remove_deletesAndSignals();
    void remove_reportsAnUnknownId();

    void load_fillsTheEditProperties();
    void load_givesAPriceTheFieldCanParseBack();
    void load_reportsAnUnknownId();

    void lastError_clearsAfterASuccess();

private:
    int m_groceriesId = -1;
};

void TestPriceItemController::initTestCase()
{
    TestUtils::enableTestMode();
    TestUtils::isolateSettings();
}

void TestPriceItemController::init()
{
    QVERIFY(TestUtils::resetDatabase());
    m_groceriesId = TestUtils::categoryId(QStringLiteral("Groceries"));
    QVERIFY(m_groceriesId > 0);
}

void TestPriceItemController::add_storesAndSignals()
{
    PriceItemController controller;
    QSignalSpy addedSpy(&controller, &PriceItemController::priceItemAdded);

    const int id = controller.add(QStringLiteral("Milk"), m_groceriesId,
                                  QStringLiteral("litre"), QStringLiteral("45.50"));
    QVERIFY(id > 0);
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(addedSpy.at(0).at(0).toInt(), id);
    QVERIFY(controller.lastError().isEmpty());

    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->price.minorUnits(), 4550);
    QCOMPARE(stored->categoryId.value_or(-1), m_groceriesId);
}

void TestPriceItemController::add_acceptsArabicNumerals()
{
    PriceItemController controller;
    // ٤٥٫٥٠ — what an Arabic keypad sends for 45.50
    const QString typed = QStringLiteral("٤٥") + QChar(0x066B) + QStringLiteral("٥٠");
    const int id = controller.add(QStringLiteral("Milk"), m_groceriesId,
                                  QStringLiteral("litre"), typed);
    QVERIFY2(id > 0, qPrintable(controller.lastError()));

    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->price.minorUnits(), 4550);
}

void TestPriceItemController::add_acceptsZero()
{
    PriceItemController controller;
    const int id = controller.add(QStringLiteral("Bread"), -1, QString(),
                                  QStringLiteral("0"));
    QVERIFY2(id > 0, qPrintable(controller.lastError()));
}

void TestPriceItemController::add_trimsWhitespace()
{
    PriceItemController controller;
    const int id = controller.add(QStringLiteral("  Milk  "), m_groceriesId,
                                  QStringLiteral("  litre  "), QStringLiteral("45"));
    QVERIFY(id > 0);

    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->name, QStringLiteral("Milk"));
    QCOMPARE(stored->unit, QStringLiteral("litre"));
}

void TestPriceItemController::add_treatsNonPositiveCategoryAsUncategorized()
{
    PriceItemController controller;
    const int id = controller.add(QStringLiteral("Batteries"), -1,
                                  QStringLiteral("pack"), QStringLiteral("120"));
    QVERIFY(id > 0);

    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    QVERIFY(!stored->categoryId.has_value());
}

void TestPriceItemController::add_rejectsEmptyName()
{
    PriceItemController controller;
    QSignalSpy addedSpy(&controller, &PriceItemController::priceItemAdded);

    QCOMPARE(controller.add(QStringLiteral("   "), m_groceriesId,
                            QStringLiteral("kg"), QStringLiteral("45")),
             0);
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(addedSpy.count(), 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
}

void TestPriceItemController::add_rejectsUnparseablePrice()
{
    PriceItemController controller;
    QCOMPARE(controller.add(QStringLiteral("Milk"), m_groceriesId,
                            QStringLiteral("litre"), QStringLiteral("abc")),
             0);
    QVERIFY(!controller.lastError().isEmpty());

    // A negative price is not a price, however it was typed.
    QCOMPARE(controller.add(QStringLiteral("Milk"), m_groceriesId,
                            QStringLiteral("litre"), QStringLiteral("-5")),
             0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
}

void TestPriceItemController::add_rejectsDuplicateName()
{
    PriceItemController controller;
    QVERIFY(controller.add(QStringLiteral("Milk"), m_groceriesId,
                           QStringLiteral("litre"), QStringLiteral("45"))
            > 0);

    QCOMPARE(controller.add(QStringLiteral("milk"), m_groceriesId,
                            QStringLiteral("litre"), QStringLiteral("50")),
             0);
    // The message has to name the problem, not just say "failed".
    QVERIFY(controller.lastError().contains(QStringLiteral("milk")));
}

void TestPriceItemController::update_changesTheRowAndSignals()
{
    PriceItemController controller;
    const int id = controller.add(QStringLiteral("Milk"), m_groceriesId,
                                  QStringLiteral("litre"), QStringLiteral("45"));
    QVERIFY(id > 0);

    QSignalSpy updatedSpy(&controller, &PriceItemController::priceItemUpdated);
    QVERIFY(controller.update(id, QStringLiteral("Fresh milk"), -1,
                              QStringLiteral("carton"), QStringLiteral("50")));
    QCOMPARE(updatedSpy.count(), 1);
    QCOMPARE(updatedSpy.at(0).at(0).toInt(), id);

    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->name, QStringLiteral("Fresh milk"));
    QCOMPARE(stored->unit, QStringLiteral("carton"));
    QCOMPARE(stored->price.minorUnits(), 5000);
    // Clearing the category is a real edit, not a no-op.
    QVERIFY(!stored->categoryId.has_value());
}

void TestPriceItemController::update_rejectsAnUnknownId()
{
    PriceItemController controller;
    QSignalSpy updatedSpy(&controller, &PriceItemController::priceItemUpdated);

    QVERIFY(!controller.update(4242, QStringLiteral("Ghost"), -1, QString(),
                               QStringLiteral("10")));
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(updatedSpy.count(), 0);
}

void TestPriceItemController::remove_deletesAndSignals()
{
    PriceItemController controller;
    const int id = controller.add(QStringLiteral("Milk"), m_groceriesId,
                                  QStringLiteral("litre"), QStringLiteral("45"));
    QVERIFY(id > 0);

    QSignalSpy removedSpy(&controller, &PriceItemController::priceItemRemoved);
    QVERIFY(controller.remove(id));
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(0).toInt(), id);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
}

void TestPriceItemController::remove_reportsAnUnknownId()
{
    PriceItemController controller;
    QSignalSpy removedSpy(&controller, &PriceItemController::priceItemRemoved);

    QVERIFY(!controller.remove(4242));
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(removedSpy.count(), 0);
}

void TestPriceItemController::load_fillsTheEditProperties()
{
    PriceItemController controller;
    const int id = controller.add(QStringLiteral("Milk"), m_groceriesId,
                                  QStringLiteral("litre"), QStringLiteral("45.50"));
    QVERIFY(id > 0);

    QSignalSpy loadedSpy(&controller, &PriceItemController::editLoaded);
    QVERIFY(controller.load(id));
    QCOMPARE(loadedSpy.count(), 1);
    QCOMPARE(controller.editName(), QStringLiteral("Milk"));
    QCOMPARE(controller.editUnit(), QStringLiteral("litre"));
    QCOMPARE(controller.editCategoryId(), m_groceriesId);
}

void TestPriceItemController::load_givesAPriceTheFieldCanParseBack()
{
    PriceItemController controller;
    // Big enough that a grouped format would have separators in it, which
    // the amount field's parser rejects — the edit sheet would open with a
    // price it could not save.
    const int id = controller.add(QStringLiteral("Fridge"), m_groceriesId,
                                  QStringLiteral("piece"),
                                  QStringLiteral("1234567.89"));
    QVERIFY(id > 0);
    QVERIFY(controller.load(id));

    const QString text = controller.editPriceText();
    QVERIFY(!text.contains(QLatin1Char(',')));

    // Feeding it straight back must land on the same amount.
    QVERIFY(controller.update(id, QStringLiteral("Fridge"), m_groceriesId,
                              QStringLiteral("piece"), text));
    const Result<PriceItem> stored = PriceItemRepository::fetch(id);
    QVERIFY(stored.has_value());
    QCOMPARE(stored->price.minorUnits(), 123456789);
}

void TestPriceItemController::load_reportsAnUnknownId()
{
    PriceItemController controller;
    QVERIFY(!controller.load(4242));
    QVERIFY(!controller.lastError().isEmpty());
}

void TestPriceItemController::lastError_clearsAfterASuccess()
{
    PriceItemController controller;
    QSignalSpy errorSpy(&controller, &PriceItemController::lastErrorChanged);

    QCOMPARE(controller.add(QString(), m_groceriesId, QString(),
                            QStringLiteral("45")),
             0);
    QVERIFY(!controller.lastError().isEmpty());
    QCOMPARE(errorSpy.count(), 1);

    // A stale error banner over a successful save is its own bug.
    QVERIFY(controller.add(QStringLiteral("Milk"), m_groceriesId,
                           QStringLiteral("litre"), QStringLiteral("45"))
            > 0);
    QVERIFY(controller.lastError().isEmpty());
    QCOMPARE(errorSpy.count(), 2);
}

QTEST_GUILESS_MAIN(TestPriceItemController)
#include "tst_priceitemcontroller.moc"
