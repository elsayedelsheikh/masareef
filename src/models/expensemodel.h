#pragma once

#include "core/money.h"
#include "storage/expenserepository.h"

#include <QSqlQueryModel>

// Read model for the expense table view. All querying goes through
// ExpenseRepository; this class only adapts the result set to
// QAbstractItemModel (headers, amount formatting, alignment).
class ExpenseModel : public QSqlQueryModel {
    Q_OBJECT
public:
    enum Column {
        ColId = 0,
        ColDate,
        ColCategory,
        ColDescription,
        ColAmount,
        ColNotes,
        ColumnCount
    };

    explicit ExpenseModel(QObject* parent = nullptr);

    void setFilter(const ExpenseFilter& filter);
    [[nodiscard]] const ExpenseFilter& filter() const { return m_filter; }
    void refresh();

    [[nodiscard]] Money filteredTotal() const { return m_total; }
    [[nodiscard]] int expenseIdAt(int row) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    ExpenseFilter m_filter;
    Money m_total;
};
