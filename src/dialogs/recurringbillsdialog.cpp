#include "dialogs/recurringbillsdialog.h"

#include "storage/billrepository.h"
#include "storage/categoryrepository.h"
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
        const QList<Category> categories = CategoryRepository::all();
        for (const Category& cat : categories)
            m_category->addItem(cat.name, cat.id);

        m_amount = new QLineEdit(this);
        m_amount->setValidator(new QRegularExpressionValidator(
            QRegularExpression(QStringLiteral(R"(\d{1,9}([.,]\d{0,2})?)")), m_amount));

        m_recurrence = new QComboBox(this);
        for (const Recurrence recurrence :
             { Recurrence::Monthly, Recurrence::Quarterly, Recurrence::Yearly })
            m_recurrence->addItem(displayLabel(recurrence), int(recurrence));

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
            m_recurrence->setCurrentIndex(m_recurrence->findData(int(existing->recurrence)));
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
        layout->addRow(tr("Expected amount (%1):").arg(AppConfig::currencyCode()), m_amount);
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
        const std::optional<Money> amount = CurrencyFormatter::parse(m_amount->text());
        if (!amount || !amount->isPositive()) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("Please enter a valid amount greater than zero."));
            return;
        }

        m_bill.name = m_name->text().trimmed();
        m_bill.categoryId = m_category->currentData().toInt();
        m_bill.amount = *amount;
        m_bill.recurrence = Recurrence(m_recurrence->currentData().toInt());
        m_bill.nextDue = m_nextDue->date();
        m_bill.notes = m_notes->toPlainText().trimmed();

        const Result<void> saved = m_bill.id > 0
            ? BillRepository::update(m_bill)
            : BillRepository::add(m_bill).transform([](int) {});
        if (!saved) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("The bill could not be saved:\n%1")
                                     .arg(saved.error().message));
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
    const QList<RecurringBill> bills = BillRepository::all(false);
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
                         new QTableWidgetItem(displayLabel(bill.recurrence)));
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
        if (const auto bill = BillRepository::fetch(id))
            m_toggleButton->setText(bill->active ? tr("Deactivate") : tr("Activate"));
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
    if (id <= 0)
        return;
    const auto bill = BillRepository::fetch(id);
    if (!bill)
        return;
    BillEditDialog dialog(this, &*bill);
    if (dialog.exec() == QDialog::Accepted)
        reload();
}

void RecurringBillsDialog::deleteBill()
{
    const int id = selectedBillId();
    if (id <= 0)
        return;
    const auto bill = BillRepository::fetch(id);
    if (!bill)
        return;
    if (QMessageBox::question(
            this, tr("Delete Recurring Bill"),
            tr("Delete \"%1\"? Past payments stay in your expense history.").arg(bill->name))
        != QMessageBox::Yes)
        return;

    if (auto res = BillRepository::remove(id); !res) {
        QMessageBox::warning(this, tr("Delete Recurring Bill"), res.error().message);
        return;
    }
    reload();
}

void RecurringBillsDialog::toggleActive()
{
    const int id = selectedBillId();
    if (id <= 0)
        return;
    const auto bill = BillRepository::fetch(id);
    if (!bill)
        return;
    if (auto res = BillRepository::setActive(id, !bill->active); !res) {
        QMessageBox::warning(this, tr("Recurring Bills"), res.error().message);
        return;
    }
    reload();
}
