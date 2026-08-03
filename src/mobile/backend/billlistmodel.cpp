#include "backend/billlistmodel.h"

#include "utils/currencyformatter.h"
#include "utils/localeformat.h"
#include "utils/palette.h"

#include <algorithm>

namespace {

// Rounds to the nearest minor unit rather than truncating: a 100.00
// quarterly bill is 33.33 a month, and dropping the third of a piastre on
// every bill would quietly understate the total.
Money perMonth(Money amount, std::int64_t months)
{
    const std::int64_t minorUnits = amount.minorUnits();
    return Money::fromMinorUnits((minorUnits + months / 2) / months);
}

// One bill's cost expressed per month, so bills on different cadences can
// be added up into a single "this is what your bills cost" figure.
Money monthlyEquivalent(const RecurringBill& bill)
{
    switch (bill.recurrence) {
    case Recurrence::Quarterly:
        return perMonth(bill.amount, 3);
    case Recurrence::Yearly:
        return perMonth(bill.amount, 12);
    case Recurrence::Monthly:
        break;
    }
    return bill.amount;
}

} // namespace

BillListModel::BillListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    reload();
}

int BillListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant BillListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= int(m_rows.size()))
        return {};

    const auto& row = m_rows[index.row()];
    switch (role) {
    case BillIdRole:
        return row.bill.id;
    case CategoryIdRole:
        return row.bill.categoryId;
    case CategoryNameRole:
        return row.categoryName;
    case CategoryColorRole:
        return Palette::series(row.categoryColor);
    case NameRole:
        return row.bill.name;
    case AmountMinorRole:
        return qint64(row.bill.amount.minorUnits());
    case AmountFormattedRole:
        return CurrencyFormatter::format(row.bill.amount);
    case NextDueRole:
        return row.bill.nextDue;
    case NextDueFormattedRole:
        return LocaleFormat::date(row.bill.nextDue);
    case DueLabelRole:
        return LocaleFormat::dueLabel(row.bill.nextDue);
    case DaysUntilDueRole:
        return row.bill.daysUntilDue();
    case RecurrenceLabelRole:
        return displayLabel(row.bill.recurrence);
    case UrgencyRole:
        return row.urgency;
    case ActiveRole:
        return row.bill.active;
    case NotesRole:
        return row.bill.notes;
    default:
        return {};
    }
}

QHash<int, QByteArray> BillListModel::roleNames() const
{
    return {
        { BillIdRole, "billId" },
        { CategoryIdRole, "categoryId" },
        { CategoryNameRole, "categoryName" },
        { CategoryColorRole, "categoryColor" },
        { NameRole, "name" },
        { AmountMinorRole, "amountMinor" },
        { AmountFormattedRole, "amountFormatted" },
        { NextDueRole, "nextDue" },
        { NextDueFormattedRole, "nextDueFormatted" },
        { DueLabelRole, "dueLabel" },
        { DaysUntilDueRole, "daysUntilDue" },
        { RecurrenceLabelRole, "recurrenceLabel" },
        { UrgencyRole, "urgency" },
        { ActiveRole, "active" },
        { NotesRole, "notes" },
    };
}

void BillListModel::setShowPaused(bool show)
{
    if (m_showPaused == show)
        return;
    m_showPaused = show;
    emit showPausedChanged();
    refresh();
}

int BillListModel::activeCount() const
{
    return int(std::count_if(m_rows.cbegin(), m_rows.cend(),
                             [](const Row& row) { return row.bill.active; }));
}

QString BillListModel::monthlyTotalFormatted() const
{
    Money total;
    for (const Row& row : m_rows) {
        if (row.bill.active)
            total += monthlyEquivalent(row.bill);
    }
    return CurrencyFormatter::format(total);
}

void BillListModel::refresh()
{
    beginResetModel();
    reload();
    endResetModel();
    // monthlyTotalFormatted rides on countChanged, so it is emitted on every
    // refresh — an edit can move the total without changing the row count.
    emit countChanged();
}

bool BillListModel::removeAt(int row)
{
    if (row < 0 || row >= int(m_rows.size()))
        return false;

    const int billId = m_rows[row].bill.id;
    if (const auto removed = BillRepository::remove(billId); !removed)
        return false;

    beginRemoveRows({}, row, row);
    m_rows.removeAt(row);
    endRemoveRows();
    emit countChanged();
    return true;
}

int BillListModel::billIdAt(int row) const
{
    return (row >= 0 && row < int(m_rows.size())) ? m_rows[row].bill.id : -1;
}

QVariantMap BillListModel::get(int row) const
{
    if (row < 0 || row >= int(m_rows.size()))
        return {};

    QVariantMap map;
    const QModelIndex modelIndex = index(row, 0);
    const QHash<int, QByteArray> roles = roleNames();
    for (auto it = roles.cbegin(); it != roles.cend(); ++it)
        map.insert(QString::fromUtf8(it.value()), data(modelIndex, it.key()));
    return map;
}

void BillListModel::reload()
{
    m_rows.clear();
    const auto bills = BillRepository::all(!m_showPaused);

    for (const auto& bill : bills) {
        Row row;
        row.bill = bill;
        row.categoryName = bill.categoryName;
        row.categoryColor = bill.categoryColor;
        row.urgency = computeUrgency(bill);
        m_rows.append(row);
    }

    // The repository orders by due date, which already groups the active
    // rows into overdue / due / upcoming / later runs. Paused rows have a
    // due date too, so they arrive scattered through those runs — and a
    // sectioned list repeats a header every time its section reappears.
    // Moving them to the end (stably, so their own order survives) leaves
    // one contiguous run per section.
    std::stable_partition(m_rows.begin(), m_rows.end(),
                          [](const Row& row) { return row.bill.active; });
}

QString BillListModel::computeUrgency(const RecurringBill& bill)
{
    if (!bill.active)
        return QStringLiteral("paused");
    if (bill.isOverdue())
        return QStringLiteral("overdue");
    const int days = bill.daysUntilDue();
    if (days <= 7)
        return QStringLiteral("due");
    if (days <= 30)
        return QStringLiteral("upcoming");
    return QStringLiteral("later");
}
