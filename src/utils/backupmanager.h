#pragma once

#include <QString>

// Timestamped copies of the SQLite file. The startup backup runs before
// any connection is opened, so the copy is always consistent.
class BackupManager {
public:
    // Copy the DB to <data>/backups/masareef-YYYYMMDD-HHMMSS.db and keep
    // only the newest 10. No-op if the DB file doesn't exist yet.
    static void createStartupBackup();

    // File > Backup Now: closes the connection, copies to destFile, reopens.
    static bool backupTo(const QString& destFile, QString* error = nullptr);

    // File > Restore: validates srcFile is a Masareef DB, takes a safety
    // backup of the current DB, replaces the file and reopens.
    static bool restoreFrom(const QString& srcFile, QString* error = nullptr);

private:
    static bool validateBackupFile(const QString& path, QString* error);
    static void pruneOldBackups(int keep);
};
