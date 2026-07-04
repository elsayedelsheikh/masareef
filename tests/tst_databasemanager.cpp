#include "testutils.h"

#include <QtTest>

class TestDatabaseManager : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void initialize_createsSchemaAndSeeds();
    void initialize_isIdempotent();
    void upgrade_v2ToV3_migratesSystemCategories();
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

// Simulate a real pre-v3 database (a legacy 'system' row, user_version = 2)
// and verify re-initialize runs the v3 UPDATE that flips it to 'user'.
// The fresh schema already carries the v1/v2 tables, so no seed SQL is
// duplicated here — we just rewind user_version and inject a legacy row.
void TestDatabaseManager::upgrade_v2ToV3_migratesSystemCategories()
{
    QVERIFY(TestUtils::resetDatabase());

    QSqlQuery query;
    QVERIFY(query.exec(QStringLiteral(
        "INSERT INTO categories (name, type, color) "
        "VALUES ('LegacySystem', 'system', '#abcdef')")));
    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version = 2")));

    DatabaseManager::instance().close();
    VERIFY_OK(DatabaseManager::instance().initialize());

    QVERIFY(query.exec(QStringLiteral("PRAGMA user_version")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 3);

    QVERIFY(query.exec(
        QStringLiteral("SELECT COUNT(*) FROM categories WHERE type = 'system'")));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
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
