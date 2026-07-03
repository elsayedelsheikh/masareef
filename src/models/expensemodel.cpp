#include "models/expensemodel.h"

#include "utils/currencyformatter.h"

ExpenseModel::ExpenseModel(QObject* parent)
    : QSqlQueryModel(parent)
{
}

void ExpenseModel::setFilter(const ExpenseFilter& filter)
{
    m_filter = filter;
}

void ExpenseModel::refresh()
{
    setQuery(ExpenseRepository::makeFilteredQuery(m_filter));

    setHeaderData(ColDate, Qt::Horizontal, tr("Date"));
    setHeaderData(ColCategory, Qt::Horizontal, tr("Category"));
    setHeaderData(ColDescription, Qt::Horizontal, tr("Description"));
    setHeaderData(ColAmount, Qt::Horizontal, tr("Amount"));
    setHeaderData(ColNotes, Qt::Horizontal, tr("Notes"));

    m_total = ExpenseRepository::totalFor(m_filter);
}

int ExpenseModel::expenseIdAt(int row) const
{
    return QSqlQueryModel::data(index(row, ColId)).toInt();
}

QVariant ExpenseModel::data(const QModelIndex& index, int role) const
{
    if (index.column() == ColAmount) {
        if (role == Qt::DisplayRole)
            return CurrencyFormatter::format(
                Money::fromMinorUnits(QSqlQueryModel::data(index).toLongLong()));
        if (role == Qt::TextAlignmentRole)
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return QSqlQueryModel::data(index, role);
}
