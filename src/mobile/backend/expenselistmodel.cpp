#include "backend/expenselistmodel.h"

#include "storage/categoryrepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

#include <QSqlQuery>

ExpenseListModel::ExpenseListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    reload();
}

int ExpenseListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant ExpenseListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};

    const Row& row = m_rows.at(index.row());
    switch (role) {
    case ExpenseIdRole:
        return row.expense.id;
    case DateRole:
        return row.expense.date;
    case CategoryIdRole:
        return row.expense.categoryId;
    case CategoryNameRole:
        return row.categoryName;
    case CategoryColorRole:
        return Palette::series(row.categoryColor);
    case DescriptionRole:
        return row.expense.description;
    case AmountMinorRole:
        return qint64(row.expense.amount.minorUnits());
    case AmountFormattedRole:
        return CurrencyFormatter::format(row.expense.amount);
    case NotesRole:
        return row.expense.notes;
    case DateSectionRole:
        return row.expense.date.toString(Qt::ISODate);
    default:
        return {};
    }
}

QHash<int, QByteArray> ExpenseListModel::roleNames() const
{
    return {
        { ExpenseIdRole, "expenseId" },
        { DateRole, "date" },
        { CategoryIdRole, "categoryId" },
        { CategoryNameRole, "categoryName" },
        { CategoryColorRole, "categoryColor" },
        { DescriptionRole, "description" },
        { AmountMinorRole, "amountMinor" },
        { AmountFormattedRole, "amountFormatted" },
        { NotesRole, "notes" },
        { DateSectionRole, "dateSection" },
    };
}

void ExpenseListModel::setFromDate(QDate date)
{
    if (m_filter.from == date)
        return;
    m_filter.from = date;
    emit filterChanged();
    reload();
}

void ExpenseListModel::setToDate(QDate date)
{
    if (m_filter.to == date)
        return;
    m_filter.to = date;
    emit filterChanged();
    reload();
}

void ExpenseListModel::setCategoryId(int categoryId)
{
    if (m_filter.categoryId == categoryId)
        return;
    m_filter.categoryId = categoryId;
    emit filterChanged();
    reload();
}

void ExpenseListModel::setSearchText(const QString& text)
{
    if (m_filter.searchText == text)
        return;
    m_filter.searchText = text;
    emit filterChanged();
    reload();
}

QString ExpenseListModel::totalFormatted() const
{
    return CurrencyFormatter::format(m_total);
}

void ExpenseListModel::refresh()
{
    reload();
}

bool ExpenseListModel::removeAt(int row)
{
    if (row < 0 || row >= m_rows.size())
        return false;
    if (!ExpenseRepository::remove(m_rows.at(row).expense.id))
        return false;

    beginRemoveRows({}, row, row);
    m_rows.removeAt(row);
    endRemoveRows();
    emit countChanged();
    updateTotal();
    return true;
}

int ExpenseListModel::expenseIdAt(int row) const
{
    if (row < 0 || row >= m_rows.size())
        return -1;
    return m_rows.at(row).expense.id;
}

QVariantMap ExpenseListModel::get(int row) const
{
    if (row < 0 || row >= m_rows.size())
        return {};

    QVariantMap map;
    const QModelIndex index = this->index(row, 0);
    const QHash<int, QByteArray> roles = roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it)
        map.insert(QString::fromUtf8(it.value()), data(index, it.key()));
    return map;
}

void ExpenseListModel::reload()
{
    QHash<int, Category> categories;
    for (const Category& category : CategoryRepository::all())
        categories.insert(category.id, category);

    // makeFilteredQuery provides the filtered, ordered id list; fetch the
    // full rows so every role (categoryId, notes, ...) is available.
    QList<int> ids;
    QSqlQuery query = ExpenseRepository::makeFilteredQuery(m_filter);
    while (query.next())
        ids.append(query.value(0).toInt());

    QList<Row> rows;
    if (auto expenses = ExpenseRepository::fetchMany(ids)) {
        rows.reserve(expenses->size());
        for (Expense& expense : *expenses) {
            const Category category = categories.value(expense.categoryId);
            rows.append({ std::move(expense), category.name, category.color });
        }
    }

    const int oldCount = int(m_rows.size());
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
    if (oldCount != m_rows.size())
        emit countChanged();
    updateTotal();
}

void ExpenseListModel::updateTotal()
{
    const Money total = ExpenseRepository::totalFor(m_filter);
    if (total == m_total)
        return;
    m_total = total;
    emit totalChanged();
}
