#pragma once

#include "core/result.h"

#include <QSqlDatabase>
#include <QString>

// Owns the application's SQLite connection: opens the DB file under
// AppDataLocation, runs schema migrations (PRAGMA user_version) and
// seeds the system categories on first run.
class DatabaseManager {
public:
    static DatabaseManager& instance();

    [[nodiscard]] Result<void> initialize();
    void close();
    [[nodiscard]] bool isOpen() const;

    [[nodiscard]] static QString dataDirPath();
    [[nodiscard]] static QString databaseFilePath();
    [[nodiscard]] static QString backupsDirPath();

private:
    DatabaseManager() = default;
    Q_DISABLE_COPY_MOVE(DatabaseManager)

    [[nodiscard]] Result<void> migrate(QSqlDatabase& db);
};
