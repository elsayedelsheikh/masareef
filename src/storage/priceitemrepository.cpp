#include "storage/priceitemrepository.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace {

QString tr(const char* text)
{
    return QCoreApplication::translate("PriceItemRepository", text);
}

// The current price is the newest history row, so the price *before* it is
// the second newest — ordering by id as well keeps two changes made on the
// same day in the order they were made.
const QString kSelectItem = QStringLiteral(
    "SELECT p.id, p.name, p.category_id, p.unit, p.price, p.updated_at,"
    " c.name, c.color,"
    " (SELECT h.price FROM price_history h"
    "   WHERE h.price_item_id = p.id"
    "   ORDER BY h.recorded_at DESC, h.id DESC"
    "   LIMIT 1 OFFSET 1)"
    " FROM price_items p LEFT JOIN categories c ON c.id = p.category_id");

PriceItem itemFromQuery(const QSqlQuery& query)
{
    PriceItem item;
    item.id = query.value(0).toInt();
    item.name = query.value(1).toString();
    if (!query.value(2).isNull())
        item.categoryId = query.value(2).toInt();
    item.unit = query.value(3).toString();
    item.price = Money::fromMinorUnits(query.value(4).toLongLong());
    item.updatedAt = QDate::fromString(query.value(5).toString(), Qt::ISODate);
    item.categoryName = query.value(6).toString();
    item.categoryColor = query.value(7).toString();
    if (!query.value(8).isNull())
        item.previousPrice = Money::fromMinorUnits(query.value(8).toLongLong());
    return item;
}

// The name index is the only UNIQUE constraint on price_items, so a
// constraint violation there is always a duplicate name. Anything else —
// a locked file, a disk error — has to report itself, not be dressed up as
// a name clash. SQLite answers with result code 19 (SQLITE_CONSTRAINT) or
// its extended form 2067 (SQLITE_CONSTRAINT_UNIQUE) depending on the
// build, hence both.
bool isDuplicateName(const QSqlError& error)
{
    const QString code = error.nativeErrorCode();
    return code == QLatin1String("19") || code == QLatin1String("2067");
}

QVariant categoryBind(const std::optional<int>& categoryId)
{
    return categoryId ? QVariant(*categoryId) : QVariant(QMetaType(QMetaType::Int));
}

Result<void> validated(const PriceItem& item)
{
    if (item.name.trimmed().isEmpty())
        return fail(tr("The item name cannot be empty."));
    if (item.price.isNegative())
        return fail(tr("The price cannot be negative."));
    return {};
}

Result<void> recordPrice(QSqlQuery& query, int itemId, Money price, QDate when)
{
    query.prepare(QStringLiteral(
        "INSERT INTO price_history (price_item_id, price, recorded_at)"
        " VALUES (?, ?, ?)"));
    query.addBindValue(itemId);
    query.addBindValue(qint64(price.minorUnits()));
    query.addBindValue(when.toString(Qt::ISODate));
    if (!query.exec())
        return fail(query.lastError().text());
    return {};
}

} // namespace

namespace PriceItemRepository {

QList<PriceItem> all(const QString& searchText)
{
    QList<PriceItem> list;
    QSqlQuery query;

    const QString trimmed = searchText.trimmed();
    QString sql = kSelectItem;
    if (!trimmed.isEmpty())
        sql += QStringLiteral(" WHERE p.name LIKE ? OR p.unit LIKE ?");
    sql += QStringLiteral(" ORDER BY p.name COLLATE NOCASE");

    if (!query.prepare(sql))
        return list;
    if (!trimmed.isEmpty()) {
        // Built by concatenation, not arg(): QString::arg has no %% escape.
        const QString pattern = QLatin1Char('%') + trimmed + QLatin1Char('%');
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    }
    if (!query.exec())
        return list;

    while (query.next())
        list.append(itemFromQuery(query));
    return list;
}

Result<PriceItem> fetch(int id)
{
    QSqlQuery query;
    if (!query.prepare(kSelectItem + QStringLiteral(" WHERE p.id = ?")))
        return fail(query.lastError().text());
    query.addBindValue(id);
    if (!query.exec())
        return fail(query.lastError().text());
    if (!query.next())
        return fail(tr("Price book item %1 not found").arg(id));
    return itemFromQuery(query);
}

Result<int> add(const PriceItem& item)
{
    if (auto valid = validated(item); !valid)
        return std::unexpected(valid.error());

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
        return fail(db.lastError().text());

    const auto rollback = [&db](QString message) -> std::unexpected<Error> {
        db.rollback();
        return fail(std::move(message));
    };

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO price_items (name, category_id, unit, price, updated_at)"
        " VALUES (?, ?, ?, ?, ?)"));
    query.addBindValue(item.name.trimmed());
    query.addBindValue(categoryBind(item.categoryId));
    query.addBindValue(item.unit.trimmed());
    query.addBindValue(qint64(item.price.minorUnits()));
    query.addBindValue(item.updatedAt.isValid()
                           ? item.updatedAt.toString(Qt::ISODate)
                           : QDate::currentDate().toString(Qt::ISODate));
    if (!query.exec()) {
        const QSqlError error = query.lastError();
        return rollback(isDuplicateName(error)
                            ? tr("Could not add \"%1\" — the price book already "
                                 "has an item with this name.")
                                  .arg(item.name.trimmed())
                            : error.text());
    }

    const int newId = query.lastInsertId().toInt();
    if (auto recorded = recordPrice(query, newId, item.price, QDate::currentDate());
        !recorded) {
        return rollback(recorded.error().message);
    }

    if (!db.commit())
        return rollback(db.lastError().text());
    return newId;
}

Result<void> update(const PriceItem& item)
{
    if (auto valid = validated(item); !valid)
        return valid;

    const Result<PriceItem> existing = fetch(item.id);
    if (!existing)
        return std::unexpected(existing.error());

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction())
        return fail(db.lastError().text());

    const auto rollback = [&db](QString message) -> std::unexpected<Error> {
        db.rollback();
        return fail(std::move(message));
    };

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "UPDATE price_items SET name = ?, category_id = ?, unit = ?,"
        " price = ?, updated_at = ? WHERE id = ?"));
    query.addBindValue(item.name.trimmed());
    query.addBindValue(categoryBind(item.categoryId));
    query.addBindValue(item.unit.trimmed());
    query.addBindValue(qint64(item.price.minorUnits()));
    // "Updated" means the price moved. A rename or a unit fix leaves the
    // date alone, so the row keeps saying when the price was last checked
    // — unless the stored date is unreadable, which today is the only
    // chance to replace it with a real one.
    const bool priceChanged = item.price != existing->price;
    const QDate updatedAt = (priceChanged || !existing->updatedAt.isValid())
        ? QDate::currentDate()
        : existing->updatedAt;
    query.addBindValue(updatedAt.toString(Qt::ISODate));
    query.addBindValue(item.id);
    if (!query.exec()) {
        const QSqlError error = query.lastError();
        return rollback(isDuplicateName(error)
                            ? tr("Could not save \"%1\" — the price book already "
                                 "has an item with this name.")
                                  .arg(item.name.trimmed())
                            : error.text());
    }

    // Only a real price change belongs in the history; renaming an item or
    // fixing its unit must not invent a price movement.
    if (priceChanged) {
        if (auto recorded =
                recordPrice(query, item.id, item.price, QDate::currentDate());
            !recorded) {
            return rollback(recorded.error().message);
        }
    }

    if (!db.commit())
        return rollback(db.lastError().text());
    return {};
}

Result<void> remove(int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("DELETE FROM price_items WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        return fail(query.lastError().text());
    if (query.numRowsAffected() == 0)
        return fail(tr("Price book item %1 not found").arg(id));
    return {};
}

} // namespace PriceItemRepository
