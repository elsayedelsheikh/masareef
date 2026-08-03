#include "testutils.h"

#include <QtTest>

class TestDatabaseManager : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void initialize_createsSchemaAndSeeds();
    void initialize_isIdempotent();
    void upgrade_fromV2_runsEveryLaterStep();
    void foreignKeys_areEnforced();
};

void TestDatabaseManager::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestDatabaseManager::initialize_createsSchemaAndSeeds()
{
    QVERIFY(TestUtils::resetDatabase());
    QVERIFY(QFile::exists(DatabaseManager::databaseFilePath()));

    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
    QVERIFY(query.next());
    QVERIFY(query.value(0).toInt() >= 1);

    QVERIFY(query.exec(QStringLiteral("SELECT name FROM categories ORDER BY name")));
    QStringList names;
    while (query.next())
        names << query.value(0).toString();
    QCOMPARE(names, QStringList({ QStringLiteral("Bills"), QStringLiteral("Groceries"),
                                  QStringLiteral("Other") }));

    // Every seeded category carries a color for the charts
    QVERIFY(query.exec(QStringLiteral(
        "SELECT COUNT(*) FROM categories WHERE color IS NULL OR color = ''")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    // No category is undeletable by type anymore — all seeded rows are 'user'
    QVERIFY(query.exec(
        QStringLiteral("SELECT COUNT(*) FROM categories WHERE type != 'user'")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
}

void TestDatabaseManager::initialize_isIdempotent()
{
    QVERIFY(TestUtils::resetDatabase());
    VERIFY_OK(DatabaseManager::instance().initialize());
    QCOMPARE(TestUtils::countRows(QStringLiteral("categories")), 3);
}

// Simulate a real v2 database and verify re-initialize runs every step
// after it: the v3 UPDATE that flips 'system' categories to 'user', and the
// v4 CREATE TABLEs for the price book.
//
// A v2 file predates the price book, so those tables have to be dropped
// here as well as the version rewound. Leaving them in place would still
// pass for v3 (its step is an idempotent UPDATE) but not for v4, and a
// migration step is not required to be re-runnable — only to run once, in
// order, on a database that has not seen it.
void TestDatabaseManager::upgrade_fromV2_runsEveryLaterStep()
{
    QVERIFY(TestUtils::resetDatabase());

    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral("DROP TABLE price_history")));
    QVERIFY(query.exec(QStringLiteral("DROP TABLE price_items")));
    QVERIFY(query.exec(QStringLiteral(
        "INSERT INTO categories (name, type, color) "
        "VALUES ('LegacySystem', 'system', '#abcdef')")));
    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version = 2")));

    DatabaseManager::instance().close();
    VERIFY_OK(DatabaseManager::instance().initialize());

    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 4);

    // v3 ran
    QVERIFY(query.exec(
        QStringLiteral("SELECT COUNT(*) FROM categories WHERE type = 'system'")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    // v4 ran, and the existing rows survived it: the legacy category was
    // converted, not deleted, which a bare non-empty count would not catch.
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_items")), 0);
    QCOMPARE(TestUtils::countRows(QStringLiteral("price_history")), 0);
    QVERIFY(query.exec(QStringLiteral(
        "SELECT type FROM categories WHERE name = 'LegacySystem'")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toString(), QStringLiteral("user"));
}

void TestDatabaseManager::foreignKeys_areEnforced()
{
    QVERIFY(TestUtils::resetDatabase());
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO expenses (category_id, amount, expense_date) VALUES (?, 100, '2026-07-01')"));
    query.addBindValue(999999);
    QVERIFY(!query.exec());
}

QTEST_GUILESS_MAIN(TestDatabaseManager)
#include "tst_databasemanager.moc"
