#include "mainwindow.h"

#include "dialogs/addexpensedialog.h"
#include "dialogs/managecategoriesdialog.h"
#include "dialogs/recurringbillsdialog.h"
#include "dialogs/settingsdialog.h"
#include "utils/backupmanager.h"
#include "widgets/chartswidget.h"
#include "widgets/dashboardwidget.h"
#include "widgets/expenselistwidget.h"
#include "widgets/reminderswidget.h"

#include <QApplication>
#include <QDate>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QTabWidget>
#include <QToolBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Masareef — Household Expenses"));
    resize(980, 640);

    m_dashboard = new DashboardWidget(this);
    m_expenses = new ExpenseListWidget(this);
    m_charts = new ChartsWidget(this);
    m_reminders = new RemindersWidget(this);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(m_dashboard, tr("Dashboard"));
    m_tabs->addTab(m_expenses, tr("Expenses"));
    m_tabs->addTab(m_charts, tr("Charts"));
    m_remindersTabIndex = m_tabs->addTab(m_reminders, tr("Reminders"));
    setCentralWidget(m_tabs);

    connect(m_dashboard, &DashboardWidget::addExpenseRequested, this,
            &MainWindow::addExpense);
    connect(m_expenses, &ExpenseListWidget::dataChanged, this, &MainWindow::refreshAll);
    connect(m_reminders, &RemindersWidget::dataChanged, this, &MainWindow::refreshAll);
    // Charts are cheap to rebuild; redo them whenever the tab is opened so
    // they always reflect the latest data
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (m_tabs->widget(index) == m_charts)
            m_charts->refresh();
    });

    buildMenusAndToolbar();
    refreshAll();
}

void MainWindow::buildMenusAndToolbar()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Backup Now…"), this, &MainWindow::backupNow);
    fileMenu->addAction(tr("&Restore from Backup…"), this, &MainWindow::restoreFromBackup);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), QKeySequence::Quit, qApp, &QApplication::quit);

    QMenu* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    QAction* addExpenseAction =
        toolsMenu->addAction(tr("&Add Expense…"), QKeySequence(QStringLiteral("Ctrl+N")),
                             this, &MainWindow::addExpense);
    QAction* categoriesAction = toolsMenu->addAction(tr("Manage &Categories…"), this,
                                                     &MainWindow::manageCategories);
    QAction* billsAction = toolsMenu->addAction(tr("&Recurring Bills…"), this,
                                                &MainWindow::manageRecurringBills);
    toolsMenu->addSeparator();
    QAction* settingsAction =
        toolsMenu->addAction(tr("&Settings…"), this, &MainWindow::openSettings);

    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About Masareef"), this, [this] {
        QMessageBox::about(this, tr("About Masareef"),
                           tr("<b>Masareef %1</b><br>Household expense tracker.<br>"
                              "Data is stored locally in an SQLite database.")
                               .arg(QApplication::applicationVersion()));
    });

    QToolBar* toolbar = addToolBar(tr("Main"));
    toolbar->setMovable(false);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    toolbar->addAction(addExpenseAction);
    toolbar->addAction(categoriesAction);
    toolbar->addAction(billsAction);
    toolbar->addSeparator();
    toolbar->addAction(settingsAction);
}

void MainWindow::refreshAll()
{
    m_dashboard->refresh();
    m_expenses->refresh();
    m_reminders->refresh();
    if (m_tabs->currentWidget() == m_charts)
        m_charts->refresh();

    const int overdue = m_reminders->overdueCount();
    m_tabs->setTabText(m_remindersTabIndex,
                       overdue > 0 ? tr("Reminders (%1)").arg(overdue) : tr("Reminders"));
}

void MainWindow::addExpense()
{
    AddExpenseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refreshAll();
}

void MainWindow::manageCategories()
{
    ManageCategoriesDialog dialog(this);
    dialog.exec();
    refreshAll(); // category names/colors feed every view
}

void MainWindow::manageRecurringBills()
{
    RecurringBillsDialog dialog(this);
    dialog.exec();
    refreshAll();
}

void MainWindow::openSettings()
{
    SettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refreshAll(); // currency symbol is used everywhere
}

void MainWindow::backupNow()
{
    const QString suggested =
        QStringLiteral("masareef-backup-%1.db")
            .arg(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    const QString destFile = QFileDialog::getSaveFileName(
        this, tr("Backup Now"), suggested, tr("Masareef database (*.db)"));
    if (destFile.isEmpty())
        return;

    QString error;
    if (!BackupManager::backupTo(destFile, &error)) {
        QMessageBox::warning(this, tr("Backup Now"), error);
        return;
    }
    QMessageBox::information(this, tr("Backup Now"),
                             tr("Backup saved to:\n%1").arg(destFile));
}

void MainWindow::restoreFromBackup()
{
    const QString srcFile = QFileDialog::getOpenFileName(
        this, tr("Restore from Backup"), QString(), tr("Masareef database (*.db)"));
    if (srcFile.isEmpty())
        return;

    if (QMessageBox::warning(
            this, tr("Restore from Backup"),
            tr("Restoring will replace your current data with the selected backup.\n"
               "A safety copy of the current database is taken first.\n\nContinue?"),
            QMessageBox::Yes | QMessageBox::No)
        != QMessageBox::Yes)
        return;

    QString error;
    if (!BackupManager::restoreFrom(srcFile, &error)) {
        QMessageBox::warning(this, tr("Restore from Backup"), error);
        return;
    }
    refreshAll();
    QMessageBox::information(this, tr("Restore from Backup"),
                             tr("The backup was restored successfully."));
}
