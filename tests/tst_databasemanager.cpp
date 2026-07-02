#include "testutils.h"

#include <QtTest>

class TestDatabaseManager : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void initialize_createsSchemaAndSeeds();
    void initialize_isIdempotent();
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
    QCOMPARE(query.value(0).toInt(), 1);

    QVERIFY(query.exec(QStringLiteral(
        "SELECT name FROM categories WHERE type = 'system' ORDER BY name")));
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
}

void TestDatabaseManager::initialize_isIdempotent()
{
    QVERIFY(TestUtils::resetDatabase());
    QString error;
    QVERIFY2(DatabaseManager::instance().initialize(&error), qPrintable(error));
    QCOMPARE(TestUtils::countRows(QStringLiteral("categories")), 3);
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
