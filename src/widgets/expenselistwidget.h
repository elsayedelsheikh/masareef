#pragma once

#include <QWidget>

class ExpenseModel;
class QComboBox;
class QDateEdit;
class QLabel;
class QTableView;

class ExpenseListWidget : public QWidget {
    Q_OBJECT
public:
    explicit ExpenseListWidget(QWidget* parent = nullptr);

    // Repopulates the category filter and re-runs the query
    void refresh();

signals:
    void dataChanged();

private slots:
    void applyFilters();
    void resetToCurrentMonth();
    void editSelected();
    void deleteSelected();
    void showContextMenu(const QPoint& pos);

private:
    int selectedExpenseId() const;

    ExpenseModel* m_model = nullptr;
    QTableView* m_table = nullptr;
    QDateEdit* m_from = nullptr;
    QDateEdit* m_to = nullptr;
    QComboBox* m_category = nullptr;
    QLabel* m_totalLabel = nullptr;
};
