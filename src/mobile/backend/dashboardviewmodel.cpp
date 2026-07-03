#include "backend/dashboardviewmodel.h"

#include "storage/budgetrepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

namespace {
constexpr int kTopCategoryCount = 3;
} // namespace

TopCategoriesModel::TopCategoriesModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TopCategoriesModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : int(m_totals.size());
}

QVariant TopCategoriesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_totals.size())
        return {};

    const ExpenseRepository::CategoryTotal& total = m_totals.at(index.row());
    switch (role) {
    case NameRole:
        return total.name;
    case ColorRole:
        return Palette::series(total.color);
    case AmountFormattedRole:
        return CurrencyFormatter::format(total.total);
    case ShareRole:
        return m_monthTotal.isPositive()
            ? double(total.total.minorUnits()) / double(m_monthTotal.minorUnits())
            : 0.0;
    default:
        return {};
    }
}

QHash<int, QByteArray> TopCategoriesModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { ColorRole, "color" },
        { AmountFormattedRole, "amountFormatted" },
        { ShareRole, "share" },
    };
}

void TopCategoriesModel::reset(QList<ExpenseRepository::CategoryTotal> totals,
                               Money monthTotal)
{
    beginResetModel();
    m_totals = std::move(totals);
    m_monthTotal = monthTotal;
    endResetModel();
}

DashboardViewModel::DashboardViewModel(QObject* parent)
    : QObject(parent)
    , m_topCategories(new TopCategoriesModel(this))
{
    refresh();
}

QString DashboardViewModel::monthTotalFormatted() const
{
    return CurrencyFormatter::format(m_spent);
}

qint64 DashboardViewModel::budgetMinor() const
{
    return m_budget ? m_budget->minorUnits() : 0;
}

double DashboardViewModel::progress() const
{
    if (!m_budget || !m_budget->isPositive())
        return 0.0;
    const double ratio = double(m_spent.minorUnits()) / double(m_budget->minorUnits());
    return std::min(ratio, 1.0);
}

bool DashboardViewModel::overBudget() const
{
    return m_budget.has_value() && m_spent > *m_budget;
}

void DashboardViewModel::refresh()
{
    const QDate today = QDate::currentDate();
    const QDate monthStart(today.year(), today.month(), 1);
    const QDate monthEnd = monthStart.addMonths(1).addDays(-1);

    m_spent = ExpenseRepository::totalFor({ .from = monthStart, .to = monthEnd });
    m_budget = BudgetRepository::overallBudget();

    QList<ExpenseRepository::CategoryTotal> totals =
        ExpenseRepository::totalsByCategory(monthStart, monthEnd);
    if (totals.size() > kTopCategoryCount)
        totals.resize(kTopCategoryCount);
    m_topCategories->reset(std::move(totals), m_spent);

    emit changed();
}
