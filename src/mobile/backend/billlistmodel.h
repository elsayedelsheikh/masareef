#pragma once

#include "storage/billrepository.h"

#include <QAbstractListModel>
#include <QHash>
#include <QtQml/qqmlregistration.h>

// QML-facing list of recurring bills, organized by urgency sections.
// Each bill shows its urgency level (overdue, due ≤7d, due ≤30d, later),
// or "paused" when it has been deactivated.
class BillListModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    // Rows that are not paused. Differs from count only while showPaused is
    // on, which is exactly when the header must not count paused bills.
    Q_PROPERTY(int activeCount READ activeCount NOTIFY countChanged)
    // Paused bills are hidden by default; the screen offers a toggle so a
    // paused bill can be found again and resumed.
    Q_PROPERTY(bool showPaused READ showPaused WRITE setShowPaused NOTIFY showPausedChanged)
    // Every active bill's cost normalized to one month (quarterly / 3,
    // yearly / 12), so the header can answer "what do my bills cost me?".
    Q_PROPERTY(QString monthlyTotalFormatted READ monthlyTotalFormatted NOTIFY countChanged)

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
        NextDueFormattedRole,
        DueLabelRole,     // "Due tomorrow" / "Overdue by 3 days"
        DaysUntilDueRole, // negative when overdue
        RecurrenceLabelRole,
        UrgencyRole, // "overdue", "due", "upcoming", "later", "paused"
        ActiveRole,
        NotesRole,
    };
    Q_ENUM(Role)

    explicit BillListModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int count() const { return int(m_rows.size()); }
    [[nodiscard]] int activeCount() const;
    [[nodiscard]] bool showPaused() const { return m_showPaused; }
    void setShowPaused(bool show);
    [[nodiscard]] QString monthlyTotalFormatted() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool removeAt(int row);
    Q_INVOKABLE int billIdAt(int row) const;
    Q_INVOKABLE QVariantMap get(int row) const;

signals:
    void countChanged();
    void showPausedChanged();

private:
    struct Row {
        RecurringBill bill;
        QString categoryName;
        QString categoryColor;
        QString urgency; // "overdue", "due", "upcoming", "later", "paused"
    };

    void reload();
    [[nodiscard]] static QString computeUrgency(const RecurringBill& bill);

    QList<Row> m_rows;
    bool m_showPaused = false;
};
