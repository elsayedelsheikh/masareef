#include "backend/budgetviewmodel.h"

#include "storage/budgetrepository.h"
#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

namespace {

// Ungrouped "1234.50" for inline edit fields (formatPlain's thousands
// separators would be rejected by CurrencyFormatter::parse on save).
QString budgetText(std::optional<Money> budget)
{
    if (!budget)
        return {};
    return QStringLiteral("%1.%2")
        .arg(budget->minorUnits() / 100)
        .arg(budget->minorUnits() % 100, 2, 10, QLatin1Char('0'));
}

double progressOf(Money spent, std::optional<Money> budget)
{
    if (!budget || !budget->isPositive())
        return 0.0;
    return std::min(double(spent.minorUnits()) / double(budget->minorUnits()), 1.0);
}

} // namespace

BudgetViewModel::BudgetViewModel(QObject* parent)
    : QAbstractListModel(parent)
{
    reload();
}

int BudgetViewModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant BudgetViewModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};

    const Row& row = m_rows.at(index.row());
    switch (role) {
    case CategoryIdRole:
        return row.categoryId;
    case NameRole:
        return row.name;
    case ColorRole:
        return Palette::series(row.color);
    case BudgetMinorRole:
        return row.budget ? qint64(row.budget->minorUnits()) : qint64(0);
    case HasBudgetRole:
        return row.budget.has_value();
    case SpentMinorRole:
        return qint64(row.spent.minorUnits());
    case ProgressRole:
        return progressOf(row.spent, row.budget);
    case BudgetTextRole:
        return budgetText(row.budget);
    default:
        return {};
    }
}

QHash<int, QByteArray> BudgetViewModel::roleNames() const
{
    return {
        { CategoryIdRole, "categoryId" },
        { NameRole, "name" },
        { ColorRole, "color" },
        { BudgetMinorRole, "budgetMinor" },
        { HasBudgetRole, "hasBudget" },
        { SpentMinorRole, "spentMinor" },
        { ProgressRole, "progress" },
        { BudgetTextRole, "budgetText" },
    };
}

qint64 BudgetViewModel::overallBudgetMinor() const
{
    return m_overallBudget ? m_overallBudget->minorUnits() : 0;
}

QString BudgetViewModel::overallBudgetText() const
{
    return budgetText(m_overallBudget);
}

double BudgetViewModel::overallProgress() const
{
    return progressOf(m_overallSpent, m_overallBudget);
}

bool BudgetViewModel::setOverallBudget(const QString& amountText)
{
    const auto amount = parseBudgetText(amountText);
    if (!amount)
        return false;

    if (const auto saved = BudgetRepository::setOverallBudget(*amount); !saved) {
        setLastError(saved.error().message);
        return false;
    }
    setLastError({});
    reload();
    return true;
}

bool BudgetViewModel::setCategoryBudget(int categoryId, const QString& amountText)
{
    const auto amount = parseBudgetText(amountText);
    if (!amount)
        return false;

    if (const auto saved = BudgetRepository::setCategoryBudget(categoryId, *amount);
        !saved) {
        setLastError(saved.error().message);
        return false;
    }
    setLastError({});
    reload();
    return true;
}

void BudgetViewModel::refresh()
{
    reload();
}

std::optional<std::optional<Money>> BudgetViewModel::parseBudgetText(const QString& text)
{
    if (text.trimmed().isEmpty())
        return std::optional<Money>{}; // valid: clear the budget

    const std::optional<Money> amount = CurrencyFormatter::parse(text);
    if (!amount || !amount->isPositive()) {
        setLastError(tr("Please enter a valid amount greater than zero."));
        return std::nullopt;
    }
    return amount;
}

void BudgetViewModel::setLastError(const QString& message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}

void BudgetViewModel::reload()
{
    const QDate today = QDate::currentDate();
    const QDate monthStart(today.year(), today.month(), 1);
    const QDate monthEnd = monthStart.addMonths(1).addDays(-1);

    m_overallBudget = BudgetRepository::overallBudget();
    m_overallSpent = ExpenseRepository::totalFor({ .from = monthStart, .to = monthEnd });

    const QHash<int, Money> budgets = BudgetRepository::categoryBudgets();
    QHash<int, Money> spent;
    for (const auto& total : ExpenseRepository::totalsByCategory(monthStart, monthEnd))
        spent.insert(total.categoryId, total.total);

    QList<Row> rows;
    for (const Category& category : CategoryRepository::all()) {
        Row row{ .categoryId = category.id,
                 .name = category.name,
                 .color = category.color,
                 .spent = spent.value(category.id) };
        if (const auto it = budgets.constFind(category.id); it != budgets.constEnd())
            row.budget = it.value();
        rows.append(std::move(row));
    }

    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
    emit overallChanged();
}
