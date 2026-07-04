#include "backend/billlistmodel.h"

#include "utils/currencyformatter.h"
#include "utils/palette.h"

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
    case UrgencyRole:
        return row.urgency;
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
        { UrgencyRole, "urgency" },
        { NotesRole, "notes" },
    };
}

void BillListModel::refresh()
{
    beginResetModel();
    reload();
    endResetModel();
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

    const auto& row_ = m_rows[row];
    return {
        { QStringLiteral("billId"), row_.bill.id },
        { QStringLiteral("categoryId"), row_.bill.categoryId },
        { QStringLiteral("categoryName"), row_.categoryName },
        { QStringLiteral("categoryColor"), row_.categoryColor },
        { QStringLiteral("name"), row_.bill.name },
        { QStringLiteral("amountMinor"), qint64(row_.bill.amount.minorUnits()) },
        { QStringLiteral("amountFormatted"),
          CurrencyFormatter::format(row_.bill.amount) },
        { QStringLiteral("nextDue"), row_.bill.nextDue },
        { QStringLiteral("urgency"), row_.urgency },
        { QStringLiteral("notes"), row_.bill.notes },
    };
}

void BillListModel::reload()
{
    m_rows.clear();
    const auto bills = BillRepository::all(true); // activeOnly

    for (const auto& bill : bills) {
        Row row;
        row.bill = bill;
        row.categoryName = bill.categoryName;
        row.categoryColor = bill.categoryColor;
        row.urgency = computeUrgency(bill);
        m_rows.append(row);
    }
}

QString BillListModel::computeUrgency(const RecurringBill& bill) const
{
    if (bill.isOverdue())
        return QStringLiteral("overdue");
    const int days = bill.daysUntilDue();
    if (days <= 7)
        return QStringLiteral("due");
    if (days <= 30)
        return QStringLiteral("upcoming");
    return QStringLiteral("later");
}
