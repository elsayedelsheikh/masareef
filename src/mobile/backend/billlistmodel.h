#pragma once

#include "storage/billrepository.h"

#include <QAbstractListModel>
#include <QHash>
#include <QtQml/qqmlregistration.h>

// QML-facing list of recurring bills, organized by urgency sections.
// Each bill shows its urgency level (overdue, due ≤7d, due ≤30d, later).
class BillListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

public:
    enum Role {
        BillIdRole = Qt::UserRole + 1,
        CategoryIdRole,
        CategoryNameRole,
        CategoryColorRole,
        NameRole,
        AmountMinorRole,
        AmountFormattedRole,
        NextDueRole,
        UrgencyRole, // "overdue", "due", "upcoming", "later"
        NotesRole,
    };
    Q_ENUM(Role)

    explicit BillListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool removeAt(int row);
    Q_INVOKABLE int billIdAt(int row) const;
    Q_INVOKABLE QVariantMap get(int row) const;

private:
    struct Row {
        RecurringBill bill;
        QString categoryName;
        QString categoryColor;
        QString urgency; // "overdue", "due", "upcoming", "later"
    };

    void reload();
    [[nodiscard]] QString computeUrgency(const RecurringBill& bill) const;

    QList<Row> m_rows;
};
