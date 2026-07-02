#pragma once

#include <QSqlDatabase>
#include <QString>

// Owns the application's SQLite connection: opens the DB file under
// AppDataLocation, runs schema migrations (PRAGMA user_version) and
// seeds the system categories on first run.
class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool initialize(QString* error = nullptr);
    void close();
    bool isOpen() const;

    static QString dataDirPath();
    static QString databaseFilePath();
    static QString backupsDirPath();

private:
    DatabaseManager() = default;
    Q_DISABLE_COPY_MOVE(DatabaseManager)

    bool migrate(QSqlDatabase& db, QString* error);
};
