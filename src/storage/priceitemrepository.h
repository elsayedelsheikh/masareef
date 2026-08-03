#pragma once

#include "core/money.h"
#include "core/result.h"

#include <QDate>
#include <QList>
#include <QString>

#include <optional>

// One entry in the price book: what a thing normally costs, per unit.
//
// The category is optional — a price book entry is useful before you have
// decided which budget bucket it belongs to — which is why deleting a
// category nulls the reference instead of blocking, unlike an expense.
struct PriceItem {
    int id = 0;
    QString name;
    std::optional<int> categoryId;
    QString categoryName;  // joined, empty when uncategorized
    QString categoryColor; // joined, empty when uncategorized
    QString unit;          // free text: "kg", "L", "piece", ...
    Money price;           // per unit
    QDate updatedAt;

    // The price recorded before the current one, when there is one. Drives
    // the "up 12% since last time" hint in the UI.
    Money previousPrice;
    bool hasPreviousPrice = false;

    [[nodiscard]] bool priceRose() const
    {
        return hasPreviousPrice && price > previousPrice;
    }
    [[nodiscard]] bool priceFell() const
    {
        return hasPreviousPrice && price < previousPrice;
    }
};

// Storage access for `price_items` and its change log. Every price change
// appends to `price_history`, so the book can show which way a price has
// been moving without keeping a second source of truth.
namespace PriceItemRepository {

// Name-ordered; `searchText` matches the item name or its unit.
[[nodiscard]] QList<PriceItem> all(const QString& searchText = {});
[[nodiscard]] Result<PriceItem> fetch(int id);
[[nodiscard]] Result<int> add(const PriceItem& item); // returns the new id
[[nodiscard]] Result<void> update(const PriceItem& item);
[[nodiscard]] Result<void> remove(int id);

} // namespace PriceItemRepository
