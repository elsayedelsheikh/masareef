#include "testutils.h"

#include "storage/expenserepository.h"
#include "utils/backupmanager.h"

#include <QTemporaryDir>
#include <QtTest>

class TestBackupManager : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void startupBackup_copiesDatabase();
    void startupBackup_prunesToTen();
    void startupBackup_skipsWhenNoDatabase();
    void backupAndRestore_roundTrip();
    void restore_rejectsInvalidFile();
};

void TestBackupManager::initTestCase()
{
    TestUtils::enableTestMode();
}

void TestBackupManager::init()
{
    QVERIFY(TestUtils::resetDatabase());
}

void TestBackupManager::startupBackup_copiesDatabase()
{
    DatabaseManager::instance().close();
    BackupManager::createStartupBackup();
    const QStringList backups = QDir(DatabaseManager::backupsDirPath())
                                    .entryList({ QStringLiteral("masareef-*.db") }, QDir::Files);
    QCOMPARE(backups.size(), 1);
}

void TestBackupManager::startupBackup_prunesToTen()
{
    DatabaseManager::instance().close();
    QDir backupsDir(DatabaseManager::backupsDirPath());
    QVERIFY(QDir().mkpath(backupsDir.absolutePath()));
    for (int i = 10; i < 22; ++i) {
        QFile f(backupsDir.absoluteFilePath(
            QStringLiteral("masareef-20250101-1200%1.db").arg(i)));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
    }

    BackupManager::createStartupBackup();
    const QStringList backups =
        backupsDir.entryList({ QStringLiteral("masareef-*.db") }, QDir::Files, QDir::Name);
    QCOMPARE(backups.size(), 10);
    // The newest backup is the one just taken (today's timestamp sorts last)
    QVERIFY(backups.last() > QStringLiteral("masareef-20250101-999999.db"));
}

void TestBackupManager::startupBackup_skipsWhenNoDatabase()
{
    DatabaseManager::instance().close();
    QVERIFY(QFile::remove(DatabaseManager::databaseFilePath()));
    QDir(DatabaseManager::backupsDirPath()).removeRecursively();

    BackupManager::createStartupBackup();
    QVERIFY(!QDir(DatabaseManager::backupsDirPath()).exists()
            || QDir(DatabaseManager::backupsDirPath())
                       .entryList({ QStringLiteral("masareef-*.db") }, QDir::Files)
                       .isEmpty());
}

void TestBackupManager::backupAndRestore_roundTrip()
{
    const int billsId = TestUtils::categoryId(QStringLiteral("Bills"));
    VERIFY_OK(ExpenseRepository::add({ .categoryId = billsId,
                                       .amount = Money::fromMinorUnits(100),
                                       .description = QStringLiteral("kept"),
                                       .date = QDate(2026, 7, 1) }));

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString backupFile = tempDir.filePath(QStringLiteral("manual-backup.db"));
    VERIFY_OK(BackupManager::backupTo(backupFile));
    QVERIFY(QFile::exists(backupFile));
    QVERIFY(DatabaseManager::instance().isOpen());

    // Change the data after the backup...
    VERIFY_OK(ExpenseRepository::add({ .categoryId = billsId,
                                       .amount = Money::fromMinorUnits(999),
                                       .description = QStringLiteral("lost"),
                                       .date = QDate(2026, 7, 2) }));
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 2);

    // ...restore rolls back to the snapshot and the app keeps working
    VERIFY_OK(BackupManager::restoreFrom(backupFile));
    QVERIFY(DatabaseManager::instance().isOpen());
    QCOMPARE(TestUtils::countRows(QStringLiteral("expenses")), 1);

    // A safety copy of the pre-restore DB was kept
    const QStringList backups = QDir(DatabaseManager::backupsDirPath())
                                    .entryList({ QStringLiteral("masareef-*.db") }, QDir::Files);
    QVERIFY(!backups.isEmpty());
}

void TestBackupManager::restore_rejectsInvalidFile()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString bogus = tempDir.filePath(QStringLiteral("not-a-db.db"));
    QFile f(bogus);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("this is not a sqlite database");
    f.close();

    const Result<void> restored = BackupManager::restoreFrom(bogus);
    QVERIFY(!restored);
    QVERIFY(!restored.error().message.isEmpty());
    // Original DB is untouched and still open after the failed restore
    QCOMPARE(TestUtils::countRows(QStringLiteral("categories")), 3);
}

QTEST_GUILESS_MAIN(TestBackupManager)
#include "tst_backupmanager.moc"
