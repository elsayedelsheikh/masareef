#include "dialogs/addexpensedialog.h"

#include "models/categorymodel.h"
#include "models/expensemodel.h"
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
    layout->addRow(tr("Amount (%1):").arg(AppConfig::currencySymbol()), m_amount);
    layout->addRow(tr("Date:"), m_date);
    layout->addRow(tr("Description:"), m_description);
    layout->addRow(tr("Notes:"), m_notes);
    layout->addRow(buttons);

    m_amount->setFocus();
}

void AddExpenseDialog::populateCategories(int selectedId)
{
    m_category->clear();
    const QList<Category> categories = CategoryModel::allCategories();
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
    ExpenseModel::Expense expense;
    if (!ExpenseModel::fetchExpense(expenseId, &expense))
        return;

    m_mode = Mode::Edit;
    m_expenseId = expenseId;
    setWindowTitle(tr("Edit Expense"));
    setPreselectedCategory(expense.categoryId);
    m_amount->setText(CurrencyFormatter::formatPlain(expense.amount).remove(QLatin1Char(',')));
    m_date->setDate(expense.date);
    m_description->setText(expense.description);
    m_notes->setPlainText(expense.notes);
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

qint64 AddExpenseDialog::parsedAmount(bool* ok) const
{
    return CurrencyFormatter::parse(m_amount->text(), ok);
}

void AddExpenseDialog::accept()
{
    bool ok = false;
    const qint64 amount = parsedAmount(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, windowTitle(),
                             tr("Please enter a valid amount greater than zero."));
        m_amount->setFocus();
        return;
    }
    if (m_category->currentIndex() < 0) {
        QMessageBox::warning(this, windowTitle(), tr("Please choose a category."));
        return;
    }

    const int categoryId = m_category->currentData().toInt();
    const QDate date = m_date->date();
    const QString description = m_description->text().trimmed();
    const QString notes = m_notes->toPlainText().trimmed();

    QString error;
    bool saved = false;
    switch (m_mode) {
    case Mode::Add:
        saved = ExpenseModel::addExpense(categoryId, amount, description, date, notes,
                                         QVariant(), &error);
        break;
    case Mode::Edit:
        saved = ExpenseModel::updateExpense(m_expenseId, categoryId, amount, description,
                                            date, notes, &error);
        break;
    case Mode::MarkPaid:
        saved = RecurringBillModel::markPaid(m_bill.id, amount, date, description, notes,
                                             &error);
        break;
    }

    if (!saved) {
        QMessageBox::warning(this, windowTitle(),
                             tr("The expense could not be saved:\n%1").arg(error));
        return;
    }
    QDialog::accept();
}
