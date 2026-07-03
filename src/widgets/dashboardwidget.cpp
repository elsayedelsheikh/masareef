#include "widgets/dashboardwidget.h"

#include "storage/billrepository.h"
#include "storage/expenserepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* header = new QHBoxLayout;
    auto* title = new QLabel(tr("This month"), this);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() * 1.3);
    titleFont.setBold(true);
    title->setFont(titleFont);
    auto* addButton = new QPushButton(tr("+ Add Expense"), this);
    addButton->setDefault(true);
    connect(addButton, &QPushButton::clicked, this, &DashboardWidget::addExpenseRequested);
    header->addWidget(title);
    header->addStretch();
    header->addWidget(addButton);

    auto* cards = new QHBoxLayout;
    cards->addWidget(makeStatCard(tr("Total spent"), m_totalValue));
    cards->addWidget(makeStatCard(tr("Bills"), m_billsValue));
    cards->addWidget(makeStatCard(tr("Groceries"), m_groceriesValue));

    auto* categoryBox = new QGroupBox(tr("Spending by category"), this);
    m_categoryRows = new QVBoxLayout(categoryBox);
    m_categoryRows->setSpacing(4);

    auto* reminderBox = new QGroupBox(tr("Bills due soon"), this);
    m_reminderRows = new QVBoxLayout(reminderBox);
    m_reminderRows->setSpacing(4);

    auto* columns = new QHBoxLayout;
    columns->addWidget(categoryBox, 1);
    columns->addWidget(reminderBox, 1);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(header);
    layout->addLayout(cards);
    layout->addLayout(columns, 1);
}

QWidget* DashboardWidget::makeStatCard(const QString& title, QLabel*& valueLabel)
{
    auto* card = new QFrame(this);
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(QStringLiteral(
        "QFrame { background: %1; border: 1px solid %2; border-radius: 6px; }")
        .arg(Palette::surface().name(), Palette::gridline().name()));

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet(QStringLiteral("color: %1; border: none;")
                                  .arg(Palette::secondaryInk().name()));

    valueLabel = new QLabel(QStringLiteral("—"), card);
    QFont valueFont = valueLabel->font();
    valueFont.setPointSizeF(valueFont.pointSizeF() * 1.6);
    valueFont.setBold(true);
    valueLabel->setFont(valueFont);
    valueLabel->setStyleSheet(QStringLiteral("color: %1; border: none;")
                                  .arg(Palette::primaryInk().name()));

    auto* layout = new QVBoxLayout(card);
    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);
    return card;
}

void DashboardWidget::clearLayout(QVBoxLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
}

void DashboardWidget::refresh()
{
    const QDate today = QDate::currentDate();
    const QDate monthStart(today.year(), today.month(), 1);
    const QDate monthEnd = monthStart.addMonths(1).addDays(-1);

    m_totalValue->setText(CurrencyFormatter::format(
        ExpenseRepository::totalFor({ .from = monthStart, .to = monthEnd })));

    const QList<ExpenseRepository::CategoryTotal> totals =
        ExpenseRepository::totalsByCategory(monthStart, monthEnd);

    Money bills, groceries;
    for (const auto& t : totals) {
        if (t.name == QLatin1String("Bills"))
            bills = t.total;
        else if (t.name == QLatin1String("Groceries"))
            groceries = t.total;
    }
    m_billsValue->setText(CurrencyFormatter::format(bills));
    m_groceriesValue->setText(CurrencyFormatter::format(groceries));

    clearLayout(m_categoryRows);
    if (totals.isEmpty()) {
        auto* empty = new QLabel(tr("No expenses recorded this month yet."), this);
        empty->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::mutedInk().name()));
        m_categoryRows->addWidget(empty);
    }
    for (const auto& t : totals) {
        auto* row = new QWidget(this);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto* swatch = new QLabel(row);
        swatch->setFixedSize(10, 10);
        swatch->setStyleSheet(QStringLiteral("background: %1; border-radius: 2px;")
                                  .arg(Palette::series(t.color).name()));
        auto* name = new QLabel(t.name, row);
        auto* amount = new QLabel(CurrencyFormatter::format(t.total), row);
        rowLayout->addWidget(swatch);
        rowLayout->addWidget(name);
        rowLayout->addStretch();
        rowLayout->addWidget(amount);
        m_categoryRows->addWidget(row);
    }
    m_categoryRows->addStretch();

    clearLayout(m_reminderRows);
    const QList<RecurringBill> billsDue = BillRepository::all(true);
    int shown = 0;
    for (const RecurringBill& bill : billsDue) {
        if (bill.daysUntilDue() > 7)
            continue;
        auto* row = new QWidget(this);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto* name = new QLabel(bill.name, row);
        auto* due = new QLabel(bill.isOverdue()
                                   ? tr("overdue — was due %1").arg(bill.nextDue.toString(Qt::ISODate))
                                   : tr("due %1").arg(bill.nextDue.toString(Qt::ISODate)),
                               row);
        due->setStyleSheet(QStringLiteral("color: %1;")
                               .arg((bill.isOverdue() ? Palette::critical()
                                                      : Palette::serious()).name()));
        auto* amount = new QLabel(CurrencyFormatter::format(bill.amount), row);
        rowLayout->addWidget(name);
        rowLayout->addWidget(due);
        rowLayout->addStretch();
        rowLayout->addWidget(amount);
        m_reminderRows->addWidget(row);
        ++shown;
    }
    if (shown == 0) {
        auto* empty = new QLabel(tr("Nothing due in the next 7 days."), this);
        empty->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::mutedInk().name()));
        m_reminderRows->addWidget(empty);
    }
    m_reminderRows->addStretch();
}
