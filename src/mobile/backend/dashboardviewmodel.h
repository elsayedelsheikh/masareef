#pragma once

#include "storage/expenserepository.h"

#include <QAbstractListModel>
#include <QDate>
#include <QtQml/qqmlregistration.h>

// The Home screen's per-category spending rows: top three categories of the
// current month with their share of the month total. Owned and refreshed by
// DashboardViewModel; not registered as a standalone QML type.
class TopCategoriesModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        ColorRole,
        AmountFormattedRole,
        ShareRole,
    };

    explicit TopCategoriesModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void reset(QList<ExpenseRepository::CategoryTotal> totals, Money monthTotal);

private:
    QList<ExpenseRepository::CategoryTotal> m_totals;
    Money m_monthTotal;
};

// Month-to-date state behind the Home screen: total spent this month, the
// overall budget gauge and the top spending categories. Connect the
// ExpenseController/BudgetViewModel change signals to refresh() for live
// updates.
class DashboardViewModel : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString monthTotalFormatted READ monthTotalFormatted NOTIFY changed)
    Q_PROPERTY(qint64 spentMinor READ spentMinor NOTIFY changed)
    Q_PROPERTY(qint64 budgetMinor READ budgetMinor NOTIFY changed)
    Q_PROPERTY(bool hasBudget READ hasBudget NOTIFY changed)
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(bool overBudget READ overBudget NOTIFY changed)
    Q_PROPERTY(QAbstractItemModel* topCategories READ topCategories CONSTANT)

public:
    explicit DashboardViewModel(QObject* parent = nullptr);

    [[nodiscard]] QString monthTotalFormatted() const;
    [[nodiscard]] qint64 spentMinor() const { return m_spent.minorUnits(); }
    [[nodiscard]] qint64 budgetMinor() const;
    [[nodiscard]] bool hasBudget() const { return m_budget.has_value(); }
    [[nodiscard]] double progress() const;
    [[nodiscard]] bool overBudget() const;
    [[nodiscard]] QAbstractItemModel* topCategories() const { return m_topCategories; }

public slots:
    void refresh();

signals:
    void changed();

private:
    Money m_spent;
    std::optional<Money> m_budget;
    TopCategoriesModel* m_topCategories = nullptr;
};
