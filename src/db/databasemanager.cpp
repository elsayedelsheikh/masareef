#include "db/databasemanager.h"

#include "utils/palette.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

namespace {
constexpr int kSchemaVersion = 3;

Result<void> execOrFail(QSqlQuery& query, const QString& sql)
{
    if (query.exec(sql))
        return {};
    return fail(query.lastError().text() + QStringLiteral("\nSQL: ") + sql);
}

// Version 1: the original tables plus the seeded system categories.
const QStringList kV1Statements = {
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

// Version 2: monthly budgets. category_id NULL is the overall budget; the
// COALESCE index makes both the overall row and each per-category row
// unique (SQLite treats NULLs as distinct in a plain UNIQUE constraint).
const QStringList kV2Statements = {
    QStringLiteral(
        "CREATE TABLE budgets ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " category_id INTEGER REFERENCES categories(id) ON DELETE CASCADE,"
        " amount INTEGER NOT NULL CHECK(amount >= 0))"),
    QStringLiteral(
        "CREATE UNIQUE INDEX idx_budgets_category ON budgets(COALESCE(category_id, -1))"),
};

Result<void> seedDefaultCategories(QSqlQuery& query)
{
    const struct { const char* name; const char* color; } seeds[] = {
        { "Bills", Palette::kCategorical[0] },
        { "Groceries", Palette::kCategorical[1] },
        { "Other", Palette::kCategorical[2] },
    };
    for (const auto& seed : seeds) {
        query.prepare(QStringLiteral(
            "INSERT INTO categories (name, type, color) VALUES (?, 'user', ?)"));
        query.addBindValue(QString::fromUtf8(seed.name));
        query.addBindValue(QString::fromUtf8(seed.color));
        if (!query.exec())
            return fail(query.lastError().text());
    }
    return {};
}

// Version 3: categories are no longer split into system/user — every
// category (seeded or added) is deletable, with reassignment for in-use
// ones. The column and its CHECK stay for schema stability; existing
// 'system' rows just become 'user'.
// ponytail: the real v2->v3 upgrade path (an existing DB with 'system'
// rows) isn't covered by a test — it would need duplicating the v1/v2 seed
// SQL to build a pre-migration fixture. The statement itself is a
// no-op-safe UPDATE; add the fixture if a real migration bug ever surfaces.
const QStringList kV3Statements = {
    QStringLiteral("UPDATE categories SET type = 'user' WHERE type = 'system'"),
};
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

Result<void> DatabaseManager::initialize()
{
    if (!QDir().mkpath(dataDirPath()))
        return fail(QStringLiteral("Could not create data directory: %1").arg(dataDirPath()));

    QSqlDatabase db = QSqlDatabase::contains()
        ? QSqlDatabase::database(QSqlDatabase::defaultConnection, false)
        : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(databaseFilePath());

    if (!db.open())
        return fail(db.lastError().text());

    QSqlQuery pragma(db);
    pragma.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

    return migrate(db);
}

void DatabaseManager::close()
{
    if (QSqlDatabase::contains()) {
        QSqlDatabase db = QSqlDatabase::database(QSqlDatabase::defaultConnection, false);
        db.close();
    }
}

Result<void> DatabaseManager::migrate(QSqlDatabase& db)
{
    QSqlQuery query(db);
    if (auto res = execOrFail(query, QStringLiteral("PRAGMA user_version")); !res)
        return res;
    int version = 0;
    if (query.next())
        version = query.value(0).toInt();

    if (version >= kSchemaVersion)
        return {};

    if (!db.transaction())
        return fail(db.lastError().text());

    // Run every migration step the file hasn't seen yet, in order, inside
    // one transaction, so a half-migrated database can never be committed.
    const auto step = [&]() -> Result<void> {
        if (version < 1) {
            for (const QString& sql : kV1Statements)
                if (auto res = execOrFail(query, sql); !res)
                    return res;
            if (auto res = seedDefaultCategories(query); !res)
                return res;
        }
        if (version < 2) {
            for (const QString& sql : kV2Statements)
                if (auto res = execOrFail(query, sql); !res)
                    return res;
        }
        if (version < 3) {
            for (const QString& sql : kV3Statements)
                if (auto res = execOrFail(query, sql); !res)
                    return res;
        }
        return execOrFail(query,
                          QStringLiteral("PRAGMA user_version = %1").arg(kSchemaVersion));
    }();

    if (!step) {
        db.rollback();
        return step;
    }
    if (!db.commit()) {
        const QString message = db.lastError().text();
        db.rollback();
        return fail(message);
    }
    return {};
}
