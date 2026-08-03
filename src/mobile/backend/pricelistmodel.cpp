#include "backend/pricelistmodel.h"

#include "utils/currencyformatter.h"
#include "utils/localeformat.h"
#include "utils/palette.h"

#include <algorithm>

PriceListModel::PriceListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    reload();
}

int PriceListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : int(m_items.size());
}

QVariant PriceListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= int(m_items.size()))
        return {};

    const PriceItem& item = m_items.at(index.row());
    switch (role) {
    case PriceItemIdRole:
        return item.id;
    case NameRole:
        return item.name;
    case UnitRole:
        return item.unit;
    case PriceMinorRole:
        return qint64(item.price.minorUnits());
    case PriceFormattedRole:
        return CurrencyFormatter::format(item.price);
    case CategoryIdRole:
        return item.categoryId ? *item.categoryId : -1;
    case CategoryNameRole:
        return item.categoryName;
    case CategoryColorRole:
        // Uncategorized rows get a neutral dot rather than a bogus palette hit
        return item.categoryColor.isEmpty() ? Palette::gridline()
                                            : Palette::series(item.categoryColor);
    case HasCategoryRole:
        return item.categoryId.has_value();
    case UpdatedAtFormattedRole:
        return LocaleFormat::shortDate(item.updatedAt);
    case HasPreviousPriceRole:
        return item.previousPrice.has_value();
    case PreviousPriceFormattedRole:
        return item.previousPrice ? CurrencyFormatter::format(*item.previousPrice)
                                  : QString();
    case PriceRoseRole:
        return item.priceRose();
    case PriceFellRole:
        return item.priceFell();
    case ChangePercentRole: {
        if (!item.previousPrice || item.previousPrice->isZero())
            return 0.0;
        const double previous = double(item.previousPrice->minorUnits());
        const double current = double(item.price.minorUnits());
        return (current - previous) / previous * 100.0;
    }
    default:
        return {};
    }
}

QHash<int, QByteArray> PriceListModel::roleNames() const
{
    return {
        { PriceItemIdRole, "priceItemId" },
        { NameRole, "name" },
        { UnitRole, "unit" },
        { PriceMinorRole, "priceMinor" },
        { PriceFormattedRole, "priceFormatted" },
        { CategoryIdRole, "categoryId" },
        { CategoryNameRole, "categoryName" },
        { CategoryColorRole, "categoryColor" },
        { HasCategoryRole, "hasCategory" },
        { UpdatedAtFormattedRole, "updatedAtFormatted" },
        { HasPreviousPriceRole, "hasPreviousPrice" },
        { PreviousPriceFormattedRole, "previousPriceFormatted" },
        { PriceRoseRole, "priceRose" },
        { PriceFellRole, "priceFell" },
        { ChangePercentRole, "changePercent" },
    };
}

int PriceListModel::risenCount() const
{
    return int(std::count_if(m_items.cbegin(), m_items.cend(),
                             [](const PriceItem& item) { return item.priceRose(); }));
}

int PriceListModel::fallenCount() const
{
    return int(std::count_if(m_items.cbegin(), m_items.cend(),
                             [](const PriceItem& item) { return item.priceFell(); }));
}

void PriceListModel::setSearchText(const QString& text)
{
    if (m_searchText == text)
        return;
    m_searchText = text;
    emit searchTextChanged();
    refresh();
}

void PriceListModel::refresh()
{
    beginResetModel();
    reload();
    endResetModel();
    emit countChanged();
}

int PriceListModel::priceItemIdAt(int row) const
{
    return (row >= 0 && row < int(m_items.size())) ? m_items.at(row).id : -1;
}

QVariantMap PriceListModel::get(int row) const
{
    if (row < 0 || row >= int(m_items.size()))
        return {};

    QVariantMap map;
    const QModelIndex modelIndex = index(row, 0);
    const QHash<int, QByteArray> roles = roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it)
        map.insert(QString::fromUtf8(it.value()), data(modelIndex, it.key()));
    return map;
}

void PriceListModel::reload()
{
    m_items = PriceItemRepository::all(m_searchText);
}
