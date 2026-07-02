#pragma once

#include "db/databasemanager.h"

#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

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
    QString error;
    const bool ok = DatabaseManager::instance().initialize(&error);
    if (!ok)
        qWarning("resetDatabase failed: %s", qPrintable(error));
    return ok;
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
