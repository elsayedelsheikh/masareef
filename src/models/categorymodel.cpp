#include "models/categorymodel.h"

#include "utils/palette.h"

#include <QColor>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>

namespace {
enum TableColumn { TcId = 0, TcName, TcType, TcColor };

bool reportError(const QSqlQuery& query, QString* error)
{
    if (error)
        *error = query.lastError().text();
    return false;
}
} // namespace

CategoryModel::CategoryModel(QObject* parent)
    : QSqlTableModel(parent)
{
    setTable(QStringLiteral("categories"));
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setSort(TcName, Qt::AscendingOrder);
    select();
}

void CategoryModel::reload()
{
    select();
}

QVariant CategoryModel::data(const QModelIndex& index, int role) const
{
    // Color swatch next to the category name
    if (index.column() == TcName && role == Qt::DecorationRole) {
        const QString color =
            QSqlTableModel::data(this->index(index.row(), TcColor)).toString();
        if (!color.isEmpty())
            return QColor(color);
    }
    return QSqlTableModel::data(index, role);
}

Category CategoryModel::categoryAt(int row) const
{
    Category cat;
    cat.id = QSqlTableModel::data(index(row, TcId)).toInt();
    cat.name = QSqlTableModel::data(index(row, TcName)).toString();
    cat.type = QSqlTableModel::data(index(row, TcType)).toString();
    cat.color = QSqlTableModel::data(index(row, TcColor)).toString();
    return cat;
}

QList<Category> CategoryModel::allCategories()
{
    QList<Category> list;
    QSqlQuery query;
    if (!query.exec(QStringLiteral("SELECT id, name, type, color FROM categories ORDER BY name")))
        return list;
    while (query.next()) {
        Category cat;
        cat.id = query.value(0).toInt();
        cat.name = query.value(1).toString();
        cat.type = query.value(2).toString();
        cat.color = query.value(3).toString();
        list.append(cat);
    }
    return list;
}

bool CategoryModel::addCategory(const QString& name, const QString& color, QString* error)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        if (error)
            *error = tr("The category name cannot be empty.");
        return false;
    }
    QSqlQuery query;
    query.prepare(QStringLiteral(
        "INSERT INTO categories (name, type, color) VALUES (?, 'user', ?)"));
    query.addBindValue(trimmed);
    query.addBindValue(color);
    if (!query.exec()) {
        if (error)
            *error = tr("Could not add \"%1\" — a category with this name may already "
                        "exist.").arg(trimmed);
        return false;
    }
    return true;
}

bool CategoryModel::renameCategory(int id, const QString& newName, QString* error)
{
    const QString trimmed = newName.trimmed();
    if (trimmed.isEmpty()) {
        if (error)
            *error = tr("The category name cannot be empty.");
        return false;
    }
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE categories SET name = ? WHERE id = ?"));
    query.addBindValue(trimmed);
    query.addBindValue(id);
    if (!query.exec()) {
        if (error)
            *error = tr("Could not rename — a category named \"%1\" may already "
                        "exist.").arg(trimmed);
        return false;
    }
    return true;
}

bool CategoryModel::setColor(int id, const QString& color, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE categories SET color = ? WHERE id = ?"));
    query.addBindValue(color);
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

bool CategoryModel::isInUse(int id)
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

bool CategoryModel::removeCategory(int id, QString* error)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT type, name FROM categories WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec() || !query.next()) {
        if (error)
            *error = tr("Category not found.");
        return false;
    }
    const QString type = query.value(0).toString();
    const QString name = query.value(1).toString();

    if (type == QLatin1String("system")) {
        if (error)
            *error = tr("\"%1\" is a built-in category and cannot be deleted.").arg(name);
        return false;
    }
    if (isInUse(id)) {
        if (error)
            *error = tr("\"%1\" is used by existing expenses or recurring bills. "
                        "Reassign or delete those entries first.").arg(name);
        return false;
    }

    query.prepare(QStringLiteral("DELETE FROM categories WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return reportError(query, error);
    return true;
}

QString CategoryModel::suggestedColor()
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
