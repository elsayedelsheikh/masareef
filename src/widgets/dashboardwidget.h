#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;

// Landing tab: this-month stat cards, per-category breakdown and the
// next-7-days bill reminders, plus a quick Add Expense button.
class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(QWidget* parent = nullptr);

    void refresh();

signals:
    void addExpenseRequested();

private:
    QWidget* makeStatCard(const QString& title, QLabel*& valueLabel);
    static void clearLayout(QVBoxLayout* layout);

    QLabel* m_totalValue = nullptr;
    QLabel* m_billsValue = nullptr;
    QLabel* m_groceriesValue = nullptr;
    QVBoxLayout* m_categoryRows = nullptr;
    QVBoxLayout* m_reminderRows = nullptr;
};
