#include "backend/priceitemcontroller.h"

#include "storage/priceitemrepository.h"
#include "utils/currencyformatter.h"
#include "utils/localeformat.h"

namespace {

// Shared validation for add and update; returns the item (without id) or
// the error to surface.
Result<PriceItem> validated(const QString& name, int categoryId,
                            const QString& unit, const QString& priceText)
{
    if (name.trimmed().isEmpty())
        return fail(PriceItemController::tr("Please enter an item name."));

    const std::optional<Money> price = CurrencyFormatter::parse(priceText);
    if (!price)
        return fail(PriceItemController::tr("Please enter a valid price."));

    PriceItem item;
    item.name = name.trimmed();
    item.unit = unit.trimmed();
    item.price = *price;
    if (categoryId > 0)
        item.categoryId = categoryId;
    return item;
}

} // namespace

PriceItemController::PriceItemController(QObject* parent)
    : QObject(parent)
{
}

int PriceItemController::add(const QString& name, int categoryId,
                             const QString& unit, const QString& priceText)
{
    Result<PriceItem> item = validated(name, categoryId, unit, priceText);
    if (!item) {
        setLastError(item.error().message);
        return 0;
    }

    const Result<int> added = PriceItemRepository::add(*item);
    if (!added) {
        setLastError(added.error().message);
        return 0;
    }
    setLastError({});
    emit priceItemAdded(*added);
    return *added;
}

bool PriceItemController::update(int id, const QString& name, int categoryId,
                                 const QString& unit, const QString& priceText)
{
    Result<PriceItem> item = validated(name, categoryId, unit, priceText);
    if (!item) {
        setLastError(item.error().message);
        return false;
    }
    item->id = id;

    if (const auto updated = PriceItemRepository::update(*item); !updated) {
        setLastError(updated.error().message);
        return false;
    }
    setLastError({});
    emit priceItemUpdated(id);
    return true;
}

bool PriceItemController::remove(int id)
{
    if (const auto removed = PriceItemRepository::remove(id); !removed) {
        setLastError(removed.error().message);
        return false;
    }
    setLastError({});
    emit priceItemRemoved(id);
    return true;
}

bool PriceItemController::load(int id)
{
    const Result<PriceItem> item = PriceItemRepository::fetch(id);
    if (!item) {
        setLastError(item.error().message);
        return false;
    }

    m_editName = item->name;
    m_editUnit = item->unit;
    m_editPriceText = LocaleFormat::amountForEditing(item->price);
    m_editCategoryId = item->categoryId ? *item->categoryId : -1;
    setLastError({});
    emit editLoaded();
    return true;
}

void PriceItemController::setLastError(const QString& message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}
