#include "dialogs/recurringbillsdialog.h"

#include "models/categorymodel.h"
#include "models/recurringbillmodel.h"
#include "utils/appconfig.h"
#include "utils/currencyformatter.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {

// Small inline form for creating/editing one recurring bill template
class BillEditDialog : public QDialog {
public:
    explicit BillEditDialog(QWidget* parent, const RecurringBill* existing = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(existing ? tr("Edit Recurring Bill") : tr("Add Recurring Bill"));
        setMinimumWidth(360);

        m_name = new QLineEdit(this);
        m_category = new QComboBox(this);
        const QList<Category> categories = CategoryModel::allCategories();
        for (const Category& cat : categories)
            m_category->addItem(cat.name, cat.id);

        m_amount = new QLineEdit(this);
        m_amount->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral(R"(\d{1,9}([.,]\d{0,2})?)")), m_amount));

        m_recurrence = new QComboBox(this);
        m_recurrence->addItem(tr("Monthly"), QStringLiteral("monthly"));
        m_recurrence->addItem(tr("Quarterly"), QStringLiteral("quarterly"));
        m_recurrence->addItem(tr("Yearly"), QStringLiteral("yearly"));

        m_nextDue = new QDateEdit(QDate::currentDate(), this);
        m_nextDue->setCalendarPopup(true);
        m_nextDue->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

        m_notes = new QPlainTextEdit(this);
        m_notes->setMaximumHeight(60);

        if (existing) {
            m_bill = *existing;
            m_name->setText(existing->name);
            m_category->setCurrentIndex(m_category->findData(existing->categoryId));
            m_amount->setText(
                CurrencyFormatter::formatPlain(existing->amount).remove(QLatin1Char(',')));
            m_recurrence->setCurrentIndex(m_recurrence->findData(existing->recurrence));
            m_nextDue->setDate(existing->nextDue);
            m_notes->setPlainText(existing->notes);
        }

        auto* buttons =
            new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto* layout = new QFormLayout(this);
        layout->addRow(tr("Bill name:"), m_name);
        layout->addRow(tr("Category:"), m_category);
        layout->addRow(tr("Expected amount (%1):").arg(AppConfig::currencySymbol()), m_amount);
        layout->addRow(tr("Repeats:"), m_recurrence);
        layout->addRow(tr("Next due date:"), m_nextDue);
        layout->addRow(tr("Notes:"), m_notes);
        layout->addRow(buttons);
    }

    void accept() override
    {
        if (m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, windowTitle(), tr("Please enter a bill name."));
            return;
        }
        bool ok = false;
        const qint64 amount = CurrencyFormatter::parse(m_amount->text(), &ok);
        if (!ok || amount <= 0) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("Please enter a valid amount greater than zero."));
            return;
        }

        m_bill.name = m_name->text().trimmed();
        m_bill.categoryId = m_category->currentData().toInt();
        m_bill.amount = amount;
        m_bill.recurrence = m_recurrence->currentData().toString();
        m_bill.nextDue = m_nextDue->date();
        m_bill.notes = m_notes->toPlainText().trimmed();

        QString error;
        const bool saved = m_bill.id > 0 ? RecurringBillModel::updateBill(m_bill, &error)
                                         : RecurringBillModel::addBill(m_bill, &error);
        if (!saved) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("The bill could not be saved:\n%1").arg(error));
            return;
        }
        QDialog::accept();
    }

private:
    RecurringBill m_bill;
    QLineEdit* m_name;
    QComboBox* m_category;
    QLineEdit* m_amount;
    QComboBox* m_recurrence;
    QDateEdit* m_nextDue;
    QPlainTextEdit* m_notes;
};

enum TableColumn { TcName = 0, TcCategory, TcAmount, TcRecurrence, TcNextDue, TcActive };

} // namespace

RecurringBillsDialog::RecurringBillsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Recurring Bills"));
    setMinimumSize(560, 360);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(6);
    m_table->setHorizontalHeaderLabels({ tr("Name"), tr("Category"), tr("Amount"),
                                         tr("Repeats"), tr("Next due"), tr("Active") });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(TcName, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);

    auto* addButton = new QPushButton(tr("Add…"), this);
    m_editButton = new QPushButton(tr("Edit…"), this);
    m_deleteButton = new QPushButton(tr("Delete"), this);
    m_toggleButton = new QPushButton(tr("Deactivate"), this);

    connect(addButton, &QPushButton::clicked, this, &RecurringBillsDialog::addBill);
    connect(m_editButton, &QPushButton::clicked, this, &RecurringBillsDialog::editBill);
    connect(m_deleteButton, &QPushButton::clicked, this, &RecurringBillsDialog::deleteBill);
    connect(m_toggleButton, &QPushButton::clicked, this, &RecurringBillsDialog::toggleActive);
    connect(m_table, &QTableWidget::itemSelectionChanged, this,
            &RecurringBillsDialog::updateButtonStates);
    connect(m_table, &QTableWidget::itemDoubleClicked, this, &RecurringBillsDialog::editBill);

    auto* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(addButton);
    buttonRow->addWidget(m_editButton);
    buttonRow->addWidget(m_deleteButton);
    buttonRow->addWidget(m_toggleButton);
    buttonRow->addStretch();

    auto* closeBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(closeBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(buttonRow);
    layout->addWidget(m_table, 1);
    layout->addWidget(closeBox);

    reload();
}

void RecurringBillsDialog::reload()
{
    const QList<RecurringBill> bills = RecurringBillModel::bills(false);
    m_table->setRowCount(bills.size());
    for (int row = 0; row < bills.size(); ++row) {
        const RecurringBill& bill = bills.at(row);
        auto* nameItem = new QTableWidgetItem(bill.name);
        nameItem->setData(Qt::UserRole, bill.id);
        m_table->setItem(row, TcName, nameItem);
        m_table->setItem(row, TcCategory, new QTableWidgetItem(bill.categoryName));
        auto* amountItem = new QTableWidgetItem(CurrencyFormatter::format(bill.amount));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_table->setItem(row, TcAmount, amountItem);
        m_table->setItem(row, TcRecurrence,
                         new QTableWidgetItem(RecurringBillModel::recurrenceLabel(bill.recurrence)));
        m_table->setItem(row, TcNextDue,
                         new QTableWidgetItem(bill.nextDue.toString(Qt::ISODate)));
        m_table->setItem(row, TcActive,
                         new QTableWidgetItem(bill.active ? tr("Yes") : tr("No")));
    }
    updateButtonStates();
}

int RecurringBillsDialog::selectedBillId() const
{
    const int row = m_table->currentRow();
    if (row < 0)
        return -1;
    const QTableWidgetItem* item = m_table->item(row, TcName);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

void RecurringBillsDialog::updateButtonStates()
{
    const int id = selectedBillId();
    const bool hasSelection = id > 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_toggleButton->setEnabled(hasSelection);
    if (hasSelection) {
        RecurringBill bill;
        if (RecurringBillModel::fetchBill(id, &bill))
            m_toggleButton->setText(bill.active ? tr("Deactivate") : tr("Activate"));
    }
}

void RecurringBillsDialog::addBill()
{
    BillEditDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        reload();
}

void RecurringBillsDialog::editBill()
{
    const int id = selectedBillId();
    RecurringBill bill;
    if (id <= 0 || !RecurringBillModel::fetchBill(id, &bill))
        return;
    BillEditDialog dialog(this, &bill);
    if (dialog.exec() == QDialog::Accepted)
        reload();
}

void RecurringBillsDialog::deleteBill()
{
    const int id = selectedBillId();
    RecurringBill bill;
    if (id <= 0 || !RecurringBillModel::fetchBill(id, &bill))
        return;
    if (QMessageBox::question(
            this, tr("Delete Recurring Bill"),
            tr("Delete \"%1\"? Past payments stay in your expense history.").arg(bill.name))
        != QMessageBox::Yes)
        return;

    QString error;
    if (!RecurringBillModel::removeBill(id, &error)) {
        QMessageBox::warning(this, tr("Delete Recurring Bill"), error);
        return;
    }
    reload();
}

void RecurringBillsDialog::toggleActive()
{
    const int id = selectedBillId();
    RecurringBill bill;
    if (id <= 0 || !RecurringBillModel::fetchBill(id, &bill))
        return;
    QString error;
    if (!RecurringBillModel::setActive(id, !bill.active, &error)) {
        QMessageBox::warning(this, tr("Recurring Bills"), error);
        return;
    }
    reload();
}
