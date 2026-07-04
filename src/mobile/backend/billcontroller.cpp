#include "backend/billcontroller.h"

#include "storage/billrepository.h"
#include "utils/currencyformatter.h"

namespace {

// Shared validation for add and update.
Result<RecurringBill> validated(int categoryId, const QString& amountText,
                                const QString& name, QDate nextDue,
                                Recurrence recurrence, const QString& notes)
{
    const std::optional<Money> amount = CurrencyFormatter::parse(amountText);
    if (!amount || !amount->isPositive())
        return fail(BillController::tr("Please enter a valid amount greater than zero."));
    if (categoryId <= 0)
        return fail(BillController::tr("Please choose a category."));
    if (!nextDue.isValid())
        return fail(BillController::tr("Please choose a due date."));
    if (name.trimmed().isEmpty())
        return fail(BillController::tr("Please enter a bill name."));

    RecurringBill bill;
    bill.categoryId = categoryId;
    bill.amount = *amount;
    bill.name = name.trimmed();
    bill.nextDue = nextDue;
    bill.recurrence = recurrence;
    bill.notes = notes.trimmed();
    return bill;
}

// Ungrouped "1234.50" for the edit field.
QString editableAmountText(Money amount)
{
    return QStringLiteral("%1.%2")
        .arg(amount.minorUnits() / 100)
        .arg(amount.minorUnits() % 100, 2, 10, QLatin1Char('0'));
}

} // namespace

BillController::BillController(QObject* parent)
    : QObject(parent)
{
}

int BillController::add(int categoryId, const QString& amountText, const QString& name,
                        QDate nextDue, Recurrence recurrence, const QString& notes)
{
    Result<RecurringBill> bill = validated(categoryId, amountText, name, nextDue,
                                           recurrence, notes);
    if (!bill) {
        setLastError(bill.error().message);
        return 0;
    }

    const Result<int> added = BillRepository::add(*bill);
    if (!added) {
        setLastError(added.error().message);
        return 0;
    }
    setLastError({});
    emit billAdded(*added);
    return *added;
}

bool BillController::update(int id, int categoryId, const QString& amountText,
                            const QString& name, QDate nextDue, Recurrence recurrence,
                            const QString& notes)
{
    Result<RecurringBill> bill = validated(categoryId, amountText, name, nextDue,
                                           recurrence, notes);
    if (!bill) {
        setLastError(bill.error().message);
        return false;
    }
    bill->id = id;

    if (const auto updated = BillRepository::update(*bill); !updated) {
        setLastError(updated.error().message);
        return false;
    }
    setLastError({});
    emit billUpdated(id);
    return true;
}

bool BillController::remove(int id)
{
    if (const auto removed = BillRepository::remove(id); !removed) {
        setLastError(removed.error().message);
        return false;
    }
    setLastError({});
    emit billRemoved(id);
    return true;
}

bool BillController::markPaid(int id, const QString& amountText, QDate paidDate,
                              const QString& description)
{
    const std::optional<Money> amount = CurrencyFormatter::parse(amountText);
    if (!amount || !amount->isPositive()) {
        setLastError(tr("Please enter a valid amount greater than zero."));
        return false;
    }

    if (const auto marked = BillRepository::markPaid(id, *amount, paidDate,
                                                      description, {});
        !marked) {
        setLastError(marked.error().message);
        return false;
    }
    setLastError({});
    emit billPaid(id);
    return true;
}

bool BillController::setActive(int id, bool active)
{
    if (const auto result = BillRepository::setActive(id, active); !result) {
        setLastError(result.error().message);
        return false;
    }
    setLastError({});
    return true;
}

bool BillController::load(int id)
{
    const Result<RecurringBill> bill = BillRepository::fetch(id);
    if (!bill) {
        setLastError(bill.error().message);
        return false;
    }

    m_editCategoryId = bill->categoryId;
    m_editAmountText = editableAmountText(bill->amount);
    m_editName = bill->name;
    m_editNextDue = bill->nextDue;
    m_editRecurrence = bill->recurrence;
    m_editNotes = bill->notes;
    setLastError({});
    emit editLoaded();
    return true;
}

void BillController::setLastError(const QString& message)
{
    if (m_lastError == message)
        return;
    m_lastError = message;
    emit lastErrorChanged();
}
