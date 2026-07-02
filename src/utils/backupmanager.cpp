#include "utils/backupmanager.h"

#include "db/databasemanager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
const int kBackupsToKeep = 10;

QString timestampedBackupName()
{
    return QStringLiteral("masareef-%1.db")
        .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
}
} // namespace

void BackupManager::createStartupBackup()
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

void BackupManager::pruneOldBackups(int keep)
{
    QDir backupsDir(DatabaseManager::backupsDirPath());
    // Timestamp is embedded in the name, so name-sort == age-sort
    QStringList entries = backupsDir.entryList(
        { QStringLiteral("masareef-*.db") }, QDir::Files, QDir::Name | QDir::Reversed);
    for (int i = keep; i < entries.size(); ++i)
        backupsDir.remove(entries.at(i));
}

bool BackupManager::backupTo(const QString& destFile, QString* error)
{
    const QString dbPath = DatabaseManager::databaseFilePath();
    if (!QFile::exists(dbPath)) {
        if (error)
            *error = QStringLiteral("There is no database file to back up yet.");
        return false;
    }

    DatabaseManager::instance().close();

    bool ok = true;
    if (QFile::exists(destFile))
        ok = QFile::remove(destFile);
    if (ok)
        ok = QFile::copy(dbPath, destFile);
    if (!ok && error)
        *error = QStringLiteral("Could not write the backup file:\n%1").arg(destFile);

    QString reopenError;
    if (!DatabaseManager::instance().initialize(&reopenError)) {
        if (error)
            *error = QStringLiteral("Backup copy finished but the database could not be "
                                    "reopened:\n%1").arg(reopenError);
        return false;
    }
    return ok;
}

bool BackupManager::validateBackupFile(const QString& path, QString* error)
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
        if (!valid && error)
            *error = QStringLiteral("The selected file is not a Masareef database backup.");
        check.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return valid;
}

bool BackupManager::restoreFrom(const QString& srcFile, QString* error)
{
    if (!QFile::exists(srcFile)) {
        if (error)
            *error = QStringLiteral("Backup file not found:\n%1").arg(srcFile);
        return false;
    }

    if (!validateBackupFile(srcFile, error))
        return false;

    const QString dbPath = DatabaseManager::databaseFilePath();

    // Safety copy of the current DB before it is overwritten
    if (QFile::exists(dbPath)) {
        QDir backupsDir(DatabaseManager::backupsDirPath());
        QDir().mkpath(backupsDir.absolutePath());
        QFile::copy(dbPath, backupsDir.absoluteFilePath(timestampedBackupName()));
        pruneOldBackups(kBackupsToKeep);
    }

    DatabaseManager::instance().close();

    bool ok = true;
    if (QFile::exists(dbPath))
        ok = QFile::remove(dbPath);
    if (ok)
        ok = QFile::copy(srcFile, dbPath);
    if (!ok && error)
        *error = QStringLiteral("Could not replace the database file.");

    QString reopenError;
    if (!DatabaseManager::instance().initialize(&reopenError)) {
        if (error)
            *error = QStringLiteral("The database could not be reopened after the "
                                    "restore:\n%1").arg(reopenError);
        return false;
    }
    return ok;
}
