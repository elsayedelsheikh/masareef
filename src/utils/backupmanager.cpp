#include "utils/backupmanager.h"

#include "db/databasemanager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

namespace {
constexpr int kBackupsToKeep = 10;

QString timestampedBackupName()
{
    return QStringLiteral("masareef-%1.db")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
}

void pruneOldBackups(int keep)
{
    QDir backupsDir(DatabaseManager::backupsDirPath());
    // Timestamp is embedded in the name, so name-sort == age-sort
    QStringList entries = backupsDir.entryList(
        { QStringLiteral("masareef-*.db") }, QDir::Files, QDir::Name | QDir::Reversed);
    for (int i = keep; i < entries.size(); ++i)
        backupsDir.remove(entries.at(i));
}

Result<void> validateBackupFile(const QString& path)
{
    bool valid = false;
    const QString connName = QStringLiteral("masareef_restore_check");
    {
        QSqlDatabase check = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connName);
        check.setDatabaseName(path);
        if (check.open()) {
            QSqlQuery query(check);
            if (query.exec(QStringLiteral(
                    "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name IN "
                    "('categories','expenses','recurring_bills')"))
                && query.next() && query.value(0).toInt() == 3) {
                valid = true;
            }
        }
        check.close();
    }
    QSqlDatabase::removeDatabase(connName);
    if (!valid)
        return fail(QStringLiteral("The selected file is not a Masareef database backup."));
    return {};
}
} // namespace

namespace BackupManager {

void createStartupBackup()
{
    const QString dbPath = DatabaseManager::databaseFilePath();
    if (!QFile::exists(dbPath))
        return;

    QDir backupsDir(DatabaseManager::backupsDirPath());
    if (!QDir().mkpath(backupsDir.absolutePath()))
        return;

    const QString target = backupsDir.absoluteFilePath(timestampedBackupName());
    if (!QFile::exists(target))
        QFile::copy(dbPath, target);

    pruneOldBackups(kBackupsToKeep);
}

Result<void> backupTo(const QString& destFile)
{
    const QString dbPath = DatabaseManager::databaseFilePath();
    if (!QFile::exists(dbPath))
        return fail(QStringLiteral("There is no database file to back up yet."));

    DatabaseManager::instance().close();

    bool copied = true;
    if (QFile::exists(destFile))
        copied = QFile::remove(destFile);
    if (copied)
        copied = QFile::copy(dbPath, destFile);

    if (auto reopened = DatabaseManager::instance().initialize(); !reopened)
        return fail(QStringLiteral("Backup copy finished but the database could not be "
                                   "reopened:\n%1").arg(reopened.error().message));
    if (!copied)
        return fail(QStringLiteral("Could not write the backup file:\n%1").arg(destFile));
    return {};
}

Result<void> restoreFrom(const QString& srcFile)
{
    if (!QFile::exists(srcFile))
        return fail(QStringLiteral("Backup file not found:\n%1").arg(srcFile));

    if (auto valid = validateBackupFile(srcFile); !valid)
        return valid;

    const QString dbPath = DatabaseManager::databaseFilePath();

    // Safety copy of the current DB before it is overwritten
    if (QFile::exists(dbPath)) {
        QDir backupsDir(DatabaseManager::backupsDirPath());
        QDir().mkpath(backupsDir.absolutePath());
        QFile::copy(dbPath, backupsDir.absoluteFilePath(timestampedBackupName()));
        pruneOldBackups(kBackupsToKeep);
    }

    DatabaseManager::instance().close();

    bool copied = true;
    if (QFile::exists(dbPath))
        copied = QFile::remove(dbPath);
    if (copied)
        copied = QFile::copy(srcFile, dbPath);

    if (auto reopened = DatabaseManager::instance().initialize(); !reopened)
        return fail(QStringLiteral("The database could not be reopened after the "
                                   "restore:\n%1").arg(reopened.error().message));
    if (!copied)
        return fail(QStringLiteral("Could not replace the database file."));
    return {};
}

} // namespace BackupManager
