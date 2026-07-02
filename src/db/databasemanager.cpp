#include "db/databasemanager.h"

#include "utils/palette.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {
constexpr int kSchemaVersion = 1;

bool execOrFail(QSqlQuery& query, const QString& sql, QString* error)
{
    if (query.exec(sql))
        return true;
    if (error)
        *error = query.lastError().text() + QStringLiteral("\nSQL: ") + sql;
    return false;
}
} // namespace

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager mgr;
    return mgr;
}

QString DatabaseManager::dataDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString DatabaseManager::databaseFilePath()
{
    return dataDirPath() + QStringLiteral("/masareef.db");
}

QString DatabaseManager::backupsDirPath()
{
    return dataDirPath() + QStringLiteral("/backups");
}

bool DatabaseManager::isOpen() const
{
    return QSqlDatabase::contains() && QSqlDatabase::database().isOpen();
}

bool DatabaseManager::initialize(QString* error)
{
    if (!QDir().mkpath(dataDirPath())) {
        if (error)
            *error = QStringLiteral("Could not create data directory: %1").arg(dataDirPath());
        return false;
    }

    QSqlDatabase db = QSqlDatabase::contains()
        ? QSqlDatabase::database(QSqlDatabase::defaultConnection, false)
        : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(databaseFilePath());

    if (!db.open()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    QSqlQuery pragma(db);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    return migrate(db, error);
}

void DatabaseManager::close()
{
    if (QSqlDatabase::contains()) {
        QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
        db.close();
    }
}

bool DatabaseManager::migrate(QSqlDatabase& db, QString* error)
{
    QSqlQuery query(db);
    if (!execOrFail(query, QStringLiteral("PRAGMA user_version"), error))
        return false;
    int version = 0;
    if (query.next())
        version = query.value(0).toInt();

    if (version >= kSchemaVersion)
        return true;

    if (!db.transaction()) {
        if (error)
            *error = db.lastError().text();
        return false;
    }

    const QStringList statements = {
        QStringLiteral(
            "CREATE TABLE categories ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " name TEXT UNIQUE NOT NULL,"
            " type TEXT NOT NULL CHECK(type IN ('system','user')),"
            " color TEXT)"),
        QStringLiteral(
            "CREATE TABLE recurring_bills ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " category_id INTEGER NOT NULL REFERENCES categories(id) ON DELETE RESTRICT,"
            " name TEXT NOT NULL,"
            " amount INTEGER NOT NULL,"
            " recurrence TEXT NOT NULL CHECK(recurrence IN ('monthly','quarterly','yearly')),"
            " next_due_date DATE NOT NULL,"
            " is_active INTEGER DEFAULT 1,"
            " notes TEXT)"),
        QStringLiteral(
            "CREATE TABLE expenses ("
            " id INTEGER PRIMARY KEY AUTOINCREMENT,"
            " category_id INTEGER NOT NULL REFERENCES categories(id) ON DELETE RESTRICT,"
            " amount INTEGER NOT NULL,"
            " description TEXT,"
            " expense_date DATE NOT NULL,"
            " notes TEXT,"
            " recurring_bill_id INTEGER REFERENCES recurring_bills(id) ON DELETE SET NULL)"),
        QStringLiteral("CREATE INDEX idx_expenses_date ON expenses(expense_date)"),
        QStringLiteral("CREATE INDEX idx_expenses_category ON expenses(category_id)"),
        QStringLiteral("CREATE INDEX idx_recurring_due ON recurring_bills(next_due_date)"),
    };

    for (const QString& sql : statements) {
        if (!execOrFail(query, sql, error)) {
            db.rollback();
            return false;
        }
    }

    // Seed the undeletable system categories with fixed palette slots
    const struct { const char* name; const char* color; } seeds[] = {
        { "Bills", Palette::kCategorical[0] },
        { "Groceries", Palette::kCategorical[1] },
        { "Other", Palette::kCategorical[2] },
    };
    for (const auto& seed : seeds) {
        query.prepare(QStringLiteral(
            "INSERT INTO categories (name, type, color) VALUES (?, 'system', ?)"));
        query.addBindValue(QString::fromUtf8(seed.name));
        query.addBindValue(QString::fromUtf8(seed.color));
        if (!query.exec()) {
            if (error)
                *error = query.lastError().text();
            db.rollback();
            return false;
        }
    }

    if (!execOrFail(query, QStringLiteral("PRAGMA user_version = %1").arg(kSchemaVersion), error)) {
        db.rollback();
        return false;
    }

    if (!db.commit()) {
        if (error)
            *error = db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}
