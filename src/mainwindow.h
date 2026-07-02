#pragma once

#include <QMainWindow>

class ChartsWidget;
class DashboardWidget;
class ExpenseListWidget;
class RemindersWidget;
class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void addExpense();
    void manageCategories();
    void manageRecurringBills();
    void openSettings();
    void backupNow();
    void restoreFromBackup();
    void refreshAll();

private:
    void buildMenusAndToolbar();

    QTabWidget* m_tabs = nullptr;
    DashboardWidget* m_dashboard = nullptr;
    ExpenseListWidget* m_expenses = nullptr;
    ChartsWidget* m_charts = nullptr;
    RemindersWidget* m_reminders = nullptr;
    int m_remindersTabIndex = -1;
};
