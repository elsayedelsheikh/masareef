#include "backend/expensecontroller.h"

#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"

namespace {

// Shared validation for add and update. Returns the validated expense
// (without id) or the error to surface.
Result<Expense> validated(int categoryId, const QString& amountText,
                          const QString& description, QDate date,
                          const QString& notes)
{
    const std::optional<Money> amount = CurrencyFormatter::parse(amountText);
    if (!amount || !amount->isPositive())
        return fail(ExpenseController::tr("Please enter a valid amount greater than zero."));
    if (categoryId <= 0)
        return fail(ExpenseController::tr("Please choose a category."));
    if (!date.isValid())
        return fail(ExpenseController::tr("Please choose a date."));

    return Expense{ .categoryId = categoryId,
                    .amount = *amount,
                    .description = description.trimmed(),
                    .date = date,
                    .notes = notes.trimmed() };
}

// Ungrouped "1234.50" for the edit field — formatPlain's thousands
// separators would be rejected by CurrencyFormatter::parse on save.
QString editableAmountText(Money amount)
{
    return QStringLiteral("%1.%2")
        .arg(amount.minorUnits() / 100)
        .arg(amount.minorUnits() % 100, 2, 10, QLatin1Char('0'));
}

} // namespace

ExpenseController::ExpenseController(QObject* parent)
    : QObject(parent)
{
}

int ExpenseController::add(int categoryId, const QString& amountText,
                           const QString& description, QDate date,
                           const QString& notes)
{
    Result<Expense> expense = validated(categoryId, amountText, description, date, notes);
    if (!expense) {
        setLastError(expense.error().message);
        return 0;
    }

    const Result<int> added = ExpenseRepository::add(*expense);
    if (!added) {
        setLastError(added.error().message);
        return 0;
    }
    setLastError({});
    emit expenseAdded(*added);
    return *added;
}

bool ExpenseController::update(int id, int categoryId, const QString& amountText,
                               const QString& description, QDate date,
                               const QString& notes)
{
    Result<Expense> expense = validated(categoryId, amountText, description, date, notes);
    if (!expense) {
        setLastError(expense.error().message);
        return false;
    }
    expense->id = id;

    if (const auto updated = ExpenseRepository::update(*expense); !updated) {
        setLastError(updated.error().message);
        return false;
    }
    setLastError({});
    emit expenseUpdated(id);
    return true;
}

bool ExpenseController::remove(int id)
{
    if (const auto removed = ExpenseRepository::remove(id); !removed) {
        setLastError(removed.error().message);
        return false;
    }
    setLastError({});
    emit expenseRemoved(id);
    return true;
}

bool ExpenseController::removeMany(const QList<int>& ids)
{
    if (const auto removed = ExpenseRepository::removeMany(ids); !removed) {
        setLastError(removed.error().message);
        return false;
    }
    setLastError({});
    for (int id : ids)
        emit expenseRemoved(id);
    return true;
}

int ExpenseController::restore(int categoryId, qint64 amountMinor,
                               const QString& description, QDate date,
                               const QString& notes)
{
    const Result<int> added =
        ExpenseRepository::add({ .categoryId = categoryId,
                                 .amount = Money::fromMinorUnits(amountMinor),
                                 .description = description,
                                 .date = date,
                                 .notes = notes });
    if (!added) {
        setLastError(added.error().message);
        return 0;
    }
    setLastError({});
    emit expenseAdded(*added);
    return *added;
}

bool ExpenseController::load(int id)
{
    const Result<Expense> expense = ExpenseRepository::fetch(id);
    if (!expense) {
        setLastError(expense.error().message);
        return false;
    }

    m_editCategoryId = expense->categoryId;
    m_editAmountText = editableAmountText(expense->amount);
    m_editDescription = expense->description;
    m_editDate = expense->date;
    m_editNotes = expense->notes;
    setLastError({});
    emit editLoaded();
    return true;
}

void ExpenseController::setLastError(const QString& message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}
