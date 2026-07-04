#include "storage/categoryrepository.h"

#include "utils/palette.h"

#include <QCoreApplication>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {
QString tr(const char* text)
{
    return QCoreApplication::translate("CategoryRepository", text);
}

int categoryCount()
{
    QSqlQuery query;
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM categories")) || !query.next())
        return 0;
    return query.value(0).toInt();
}
} // namespace

namespace CategoryRepository {

QList<Category> all()
{
    QList<Category> list;
    QSqlQuery query;
    if (!query.exec(QStringLiteral("SELECT id, name, color FROM categories ORDER BY name")))
        return list;
    while (query.next()) {
        Category cat;
        cat.id = query.value(0).toInt();
        cat.name = query.value(1).toString();
        cat.color = query.value(2).toString();
        list.append(cat);
    }
    return list;
}

Result<void> add(const QString& name, const QString& color)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
        return fail(tr("The category name cannot be empty."));

    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO categories (name, type, color) VALUES (?, 'user', ?)"));
    query.addBindValue(trimmed);
    query.addBindValue(color);
    if (!query.exec())
        return fail(tr("Could not add \"%1\" — a category with this name may already "
                       "exist.").arg(trimmed));
    return {};
}

Result<void> rename(int id, const QString& newName)
{
    const QString trimmed = newName.trimmed();
    if (trimmed.isEmpty())
        return fail(tr("The category name cannot be empty."));

    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE categories SET name = ? WHERE id = ?"));
    query.addBindValue(trimmed);
    query.addBindValue(id);
    if (!query.exec())
        return fail(tr("Could not rename — a category named \"%1\" may already "
                       "exist.").arg(trimmed));
    return {};
}

Result<void> setColor(int id, const QString& color)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE categories SET color = ? WHERE id = ?"));
    query.addBindValue(color);
    query.addBindValue(id);
    if (!query.exec())
        return fail(query.lastError().text());
    return {};
}

bool isInUse(int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "SELECT EXISTS(SELECT 1 FROM expenses WHERE category_id = :id) "
        "OR EXISTS(SELECT 1 FROM recurring_bills WHERE category_id = :id)"));
    query.bindValue(QStringLiteral(":id"), id);
    if (!query.exec() || !query.next())
        return true; // fail safe: refuse deletion if the check itself failed
    return query.value(0).toBool();
}

Result<void> remove(int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT name FROM categories WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec() || !query.next())
        return fail(tr("Category not found."));
    const QString name = query.value(0).toString();

    if (categoryCount() <= 1)
        return fail(tr("At least one category must remain."));
    if (isInUse(id))
        return fail(tr("\"%1\" is used by existing expenses or recurring bills. "
                       "Reassign or delete those entries first.").arg(name));

    query.prepare(QStringLiteral("DELETE FROM categories WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return fail(query.lastError().text());
    return {};
}

Result<void> removeAndReassign(int id, int targetId)
{
    if (id == targetId)
        return fail(tr("Cannot reassign a category to itself."));

    QSqlQuery check;
    check.prepare(
        QStringLiteral("SELECT COUNT(*) FROM categories WHERE id IN (?, ?)"));
    check.addBindValue(id);
    check.addBindValue(targetId);
    if (!check.exec() || !check.next() || check.value(0).toInt() != 2)
        return fail(tr("Category not found."));

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
        return fail(db.lastError().text());

    const auto step = [&]() -> Result<void> {
        QSqlQuery query;
        query.prepare(QStringLiteral(
            "UPDATE expenses SET category_id = ? WHERE category_id = ?"));
        query.addBindValue(targetId);
        query.addBindValue(id);
        if (!query.exec())
            return fail(query.lastError().text());

        query.prepare(QStringLiteral(
            "UPDATE recurring_bills SET category_id = ? WHERE category_id = ?"));
        query.addBindValue(targetId);
        query.addBindValue(id);
        if (!query.exec())
            return fail(query.lastError().text());

        query.prepare(QStringLiteral("DELETE FROM budgets WHERE category_id = ?"));
        query.addBindValue(id);
        if (!query.exec())
            return fail(query.lastError().text());

        query.prepare(QStringLiteral("DELETE FROM categories WHERE id = ?"));
        query.addBindValue(id);
        if (!query.exec())
            return fail(query.lastError().text());
        return {};
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

QString suggestedColor()
{
    QSet<QString> used;
    QSqlQuery query;
    if (query.exec(QStringLiteral("SELECT color FROM categories"))) {
        while (query.next())
            used.insert(query.value(0).toString().toLower());
    }
    for (int i = 0; i < Palette::kCategoricalCount; ++i) {
        const QString candidate = QString::fromLatin1(Palette::kCategorical[i]);
        if (!used.contains(candidate.toLower()))
            return candidate;
    }
    // All slots taken: reuse slots in order rather than inventing new hues
    return QString::fromLatin1(
        Palette::kCategorical[used.size() % Palette::kCategoricalCount]);
}

} // namespace CategoryRepository
