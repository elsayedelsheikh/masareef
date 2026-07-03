#pragma once

#include "db/databasemanager.h"

#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>
#include <QtTest>

// QVERIFY for a Result<T>: on failure the test stops and reports the
// carried error message instead of a bare "returned FALSE".
#define VERIFY_OK(expression) \
    do { \
        if (const auto _result = (expression); !_result) \
            QFAIL(qPrintable(_result.error().message)); \
    } while (false)

// Shared fixture helpers. QStandardPaths test mode redirects
// AppDataLocation under ~/.qttest so tests never touch the real DB.
namespace TestUtils {

inline void enableTestMode()
{
    QStandardPaths::setTestModeEnabled(true);
}

inline bool resetDatabase()
{
    DatabaseManager::instance().close();
    QFile::remove(DatabaseManager::databaseFilePath());
    QDir(DatabaseManager::backupsDirPath()).removeRecursively();
    const Result<void> opened = DatabaseManager::instance().initialize();
    if (!opened)
        qWarning("resetDatabase failed: %s", qPrintable(opened.error().message));
    return opened.has_value();
}

inline int categoryId(const QString& name)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT id FROM categories WHERE name = ?"));
    query.addBindValue(name);
    if (!query.exec() || !query.next())
        return -1;
    return query.value(0).toInt();
}

inline int countRows(const QString& table)
{
    QSqlQuery query;
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM %1").arg(table)) || !query.next())
        return -1;
    return query.value(0).toInt();
}

} // namespace TestUtils
