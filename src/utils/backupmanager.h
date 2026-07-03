#pragma once

#include "core/result.h"

#include <QString>

// Timestamped copies of the SQLite file. The startup backup runs before
// any connection is opened, so the copy is always consistent.
namespace BackupManager {

// Copy the DB to <data>/backups/masareef-YYYYMMDD-HHMMSS.db and keep
// only the newest 10. No-op if the DB file doesn't exist yet.
void createStartupBackup();

// File > Backup Now: closes the connection, copies to destFile, reopens.
[[nodiscard]] Result<void> backupTo(const QString& destFile);

// File > Restore: validates srcFile is a Masareef DB, takes a safety
// backup of the current DB, replaces the file and reopens.
[[nodiscard]] Result<void> restoreFrom(const QString& srcFile);

} // namespace BackupManager
