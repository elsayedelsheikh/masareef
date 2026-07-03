#include "widgets/expenselistwidget.h"

#include "dialogs/addexpensedialog.h"
#include "models/expensemodel.h"
#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"

#include <QComboBox>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTableView>
#include <QVBoxLayout>

ExpenseListWidget::ExpenseListWidget(QWidget* parent)
    : QWidget(parent)
{
    const QDate today = QDate::currentDate();
    const QDate monthStart(today.year(), today.month(), 1);

    m_from = new QDateEdit(monthStart, this);
    m_to = new QDateEdit(monthStart.addMonths(1).addDays(-1), this);
    for (QDateEdit* edit : { m_from, m_to }) {
        edit->setCalendarPopup(true);
        edit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    }

    m_category = new QComboBox(this);

    auto* thisMonthButton = new QPushButton(tr("This month"), this);
    m_totalLabel = new QLabel(this);
    QFont totalFont = m_totalLabel->font();
    totalFont.setBold(true);
    m_totalLabel->setFont(totalFont);

    auto* filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel(tr("From:"), this));
    filterRow->addWidget(m_from);
    filterRow->addWidget(new QLabel(tr("To:"), this));
    filterRow->addWidget(m_to);
    filterRow->addWidget(new QLabel(tr("Category:"), this));
    filterRow->addWidget(m_category);
    filterRow->addWidget(thisMonthButton);
    filterRow->addStretch();
    filterRow->addWidget(new QLabel(tr("Total:"), this));
    filterRow->addWidget(m_totalLabel);

    m_model = new ExpenseModel(this);
    m_table = new QTableView(this);
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->verticalHeader()->setVisible(false);

    connect(m_from, &QDateEdit::dateChanged, this, &ExpenseListWidget::applyFilters);
    connect(m_to, &QDateEdit::dateChanged, this, &ExpenseListWidget::applyFilters);
    connect(m_category, &QComboBox::currentIndexChanged, this,
            &ExpenseListWidget::applyFilters);
    connect(thisMonthButton, &QPushButton::clicked, this,
            &ExpenseListWidget::resetToCurrentMonth);
    connect(m_table, &QTableView::doubleClicked, this, &ExpenseListWidget::editSelected);
    connect(m_table, &QTableView::customContextMenuRequested, this,
            &ExpenseListWidget::showContextMenu);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(filterRow);
    layout->addWidget(m_table, 1);
}

void ExpenseListWidget::refresh()
{
    // Rebuild the category filter without firing applyFilters per item
    {
        const QSignalBlocker blocker(m_category);
        const int previousId = m_category->currentData().toInt();
        m_category->clear();
        m_category->addItem(tr("All categories"), -1);
        const QList<Category> categories = CategoryRepository::all();
        for (const Category& cat : categories)
            m_category->addItem(cat.name, cat.id);
        const int index = m_category->findData(previousId);
        m_category->setCurrentIndex(index >= 0 ? index : 0);
    }
    applyFilters();
}

void ExpenseListWidget::applyFilters()
{
    m_model->setFilter({ .from = m_from->date(),
                         .to = m_to->date(),
                         .categoryId = m_category->currentData().toInt() });
    m_model->refresh();
    m_totalLabel->setText(CurrencyFormatter::format(m_model->filteredTotal()));

    // The model has no columns until the first query has run, so the
    // header setup has to happen here rather than in the constructor
    if (m_model->columnCount() > ExpenseModel::ColNotes) {
        m_table->setColumnHidden(ExpenseModel::ColId, true);
        m_table->horizontalHeader()->setSectionResizeMode(ExpenseModel::ColDescription,
                                                          QHeaderView::Stretch);
    }
}

void ExpenseListWidget::resetToCurrentMonth()
{
    const QDate today = QDate::currentDate();
    const QDate monthStart(today.year(), today.month(), 1);
    const QSignalBlocker fromBlocker(m_from);
    const QSignalBlocker toBlocker(m_to);
    m_from->setDate(monthStart);
    m_to->setDate(monthStart.addMonths(1).addDays(-1));
    m_category->setCurrentIndex(0); // fires applyFilters unless already 0
    applyFilters();
}

int ExpenseListWidget::selectedExpenseId() const
{
    const QModelIndex current = m_table->currentIndex();
    return current.isValid() ? m_model->expenseIdAt(current.row()) : -1;
}

void ExpenseListWidget::editSelected()
{
    const int id = selectedExpenseId();
    if (id <= 0)
        return;
    AddExpenseDialog dialog(this);
    dialog.setEditMode(id);
    if (dialog.exec() == QDialog::Accepted)
        emit dataChanged();
}

void ExpenseListWidget::deleteSelected()
{
    const int id = selectedExpenseId();
    if (id <= 0)
        return;
    if (QMessageBox::question(this, tr("Delete Expense"),
                              tr("Delete the selected expense?"))
        != QMessageBox::Yes)
        return;

    if (const Result<void> removed = ExpenseRepository::remove(id); !removed) {
        QMessageBox::warning(this, tr("Delete Expense"), removed.error().message);
        return;
    }
    emit dataChanged();
}

void ExpenseListWidget::showContextMenu(const QPoint& pos)
{
    if (selectedExpenseId() <= 0)
        return;
    QMenu menu(this);
    QAction* editAction = menu.addAction(tr("Edit…"));
    QAction* deleteAction = menu.addAction(tr("Delete"));
    QAction* chosen = menu.exec(m_table->viewport()->mapToGlobal(pos));
    if (chosen == editAction)
        editSelected();
    else if (chosen == deleteAction)
        deleteSelected();
}
