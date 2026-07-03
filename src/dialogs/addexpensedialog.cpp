#include "dialogs/addexpensedialog.h"

#include "storage/categoryrepository.h"
#include "storage/expenserepository.h"
#include "utils/appconfig.h"
#include "utils/currencyformatter.h"

#include <QColor>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QRegularExpressionValidator>

AddExpenseDialog::AddExpenseDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Add Expense"));
    setMinimumWidth(380);

    m_category = new QComboBox(this);
    populateCategories();

    m_amount = new QLineEdit(this);
    m_amount->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QStringLiteral(R"(\d{1,9}([.,]\d{0,2})?)")), m_amount));
    m_amount->setPlaceholderText(tr("0.00"));

    m_date = new QDateEdit(QDate::currentDate(), this);
    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    m_description = new QLineEdit(this);
    m_notes = new QPlainTextEdit(this);
    m_notes->setMaximumHeight(70);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QFormLayout(this);
    layout->addRow(tr("Category:"), m_category);
    layout->addRow(tr("Amount (%1):").arg(AppConfig::currencyCode()), m_amount);
    layout->addRow(tr("Date:"), m_date);
    layout->addRow(tr("Description:"), m_description);
    layout->addRow(tr("Notes:"), m_notes);
    layout->addRow(buttons);

    m_amount->setFocus();
}

void AddExpenseDialog::populateCategories(int selectedId)
{
    m_category->clear();
    const QList<Category> categories = CategoryRepository::all();
    for (const Category& cat : categories) {
        m_category->addItem(cat.name, cat.id);
        if (!cat.color.isEmpty())
            m_category->setItemData(m_category->count() - 1, QColor(cat.color),
                                    Qt::DecorationRole);
    }
    if (selectedId > 0) {
        const int index = m_category->findData(selectedId);
        if (index >= 0)
            m_category->setCurrentIndex(index);
    }
}

void AddExpenseDialog::setPreselectedCategory(int categoryId)
{
    const int index = m_category->findData(categoryId);
    if (index >= 0)
        m_category->setCurrentIndex(index);
}

void AddExpenseDialog::setEditMode(int expenseId)
{
    const auto expense = ExpenseRepository::fetch(expenseId);
    if (!expense)
        return;

    m_mode = Mode::Edit;
    m_expenseId = expenseId;
    setWindowTitle(tr("Edit Expense"));
    setPreselectedCategory(expense->categoryId);
    m_amount->setText(
        CurrencyFormatter::formatPlain(expense->amount).remove(QLatin1Char(',')));
    m_date->setDate(expense->date);
    m_description->setText(expense->description);
    m_notes->setPlainText(expense->notes);
}

void AddExpenseDialog::setMarkPaidMode(const RecurringBill& bill)
{
    m_mode = Mode::MarkPaid;
    m_bill = bill;
    setWindowTitle(tr("Record Payment — %1").arg(bill.name));
    setPreselectedCategory(bill.categoryId);
    m_category->setEnabled(false);
    m_amount->setText(CurrencyFormatter::formatPlain(bill.amount).remove(QLatin1Char(',')));
    m_description->setText(bill.name);
    m_amount->selectAll();
}

std::optional<Money> AddExpenseDialog::parsedAmount() const
{
    return CurrencyFormatter::parse(m_amount->text());
}

void AddExpenseDialog::accept()
{
    const std::optional<Money> amount = parsedAmount();
    if (!amount || !amount->isPositive()) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Please enter a valid amount greater than zero."));
        m_amount->setFocus();
        return;
    }
    if (m_category->currentIndex() < 0) {
        QMessageBox::warning(this, windowTitle(), tr("Please choose a category."));
        return;
    }

    Expense expense;
    expense.id = m_expenseId;
    expense.categoryId = m_category->currentData().toInt();
    expense.amount = *amount;
    expense.date = m_date->date();
    expense.description = m_description->text().trimmed();
    expense.notes = m_notes->toPlainText().trimmed();

    const Result<void> saved = [&]() -> Result<void> {
        switch (m_mode) {
        case Mode::Edit:
            return ExpenseRepository::update(expense);
        case Mode::MarkPaid:
            return BillRepository::markPaid(m_bill.id, expense.amount, expense.date,
                                            expense.description, expense.notes);
        case Mode::Add:
            break;
        }
        return ExpenseRepository::add(expense).transform([](int) {});
    }();

    if (!saved) {
        QMessageBox::warning(this, windowTitle(),
                             tr("The expense could not be saved:\n%1")
                                 .arg(saved.error().message));
        return;
    }
    QDialog::accept();
}
