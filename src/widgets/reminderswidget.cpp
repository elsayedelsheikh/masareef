#include "widgets/reminderswidget.h"

#include "dialogs/addexpensedialog.h"
#include "storage/billrepository.h"
#include "utils/currencyformatter.h"
#include "utils/palette.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

RemindersWidget::RemindersWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* content = new QWidget;
    m_sections = new QVBoxLayout(content);
    m_sections->setSpacing(6);

    auto* scroll = new QScrollArea(this);
    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(scroll);
}

void RemindersWidget::refresh()
{
    while (QLayoutItem* item = m_sections->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    const QList<RecurringBill> all = BillRepository::all(true);
    QList<RecurringBill> overdue, dueSoon, dueThisMonth, later;
    for (const RecurringBill& bill : all) {
        if (bill.isOverdue())
            overdue.append(bill);
        else if (bill.daysUntilDue() <= 7)
            dueSoon.append(bill);
        else if (bill.daysUntilDue() <= 30)
            dueThisMonth.append(bill);
        else
            later.append(bill);
    }
    m_overdueCount = overdue.size();

    if (all.isEmpty()) {
        auto* empty = new QLabel(
            tr("No active recurring bills. Add your bills under Tools → Recurring Bills "
               "to get reminders here."),
            this);
        empty->setWordWrap(true);
        empty->setStyleSheet(QStringLiteral("color: %1;").arg(Palette::mutedInk().name()));
        m_sections->addWidget(empty);
    } else {
        addSection(tr("Overdue"), Palette::critical().name(), overdue);
        addSection(tr("Due in the next 7 days"), Palette::serious().name(), dueSoon);
        addSection(tr("Due in the next 30 days"), Palette::secondaryInk().name(),
                   dueThisMonth);
        addSection(tr("Later"), Palette::mutedInk().name(), later);
    }
    m_sections->addStretch();
}

void RemindersWidget::addSection(const QString& title, const QString& color,
                                 const QList<RecurringBill>& bills)
{
    if (bills.isEmpty())
        return;

    auto* header = new QLabel(title, this);
    QFont font = header->font();
    font.setBold(true);
    header->setFont(font);
    header->setStyleSheet(QStringLiteral("color: %1; margin-top: 8px;").arg(color));
    m_sections->addWidget(header);

    for (const RecurringBill& bill : bills)
        m_sections->addWidget(makeBillRow(bill, color));
}

QWidget* RemindersWidget::makeBillRow(const RecurringBill& bill, const QString& accentColor)
{
    auto* row = new QFrame(this);
    row->setFrameShape(QFrame::StyledPanel);
    row->setStyleSheet(QStringLiteral(
        "QFrame { background: %1; border: 1px solid %2; border-radius: 4px; }")
        .arg(Palette::surface().name(), Palette::gridline().name()));

    auto* name = new QLabel(bill.name, row);
    QFont nameFont = name->font();
    nameFont.setBold(true);
    name->setFont(nameFont);
    name->setStyleSheet(QStringLiteral("border: none;"));

    auto* details = new QLabel(
        tr("%1 · %2 · due %3")
            .arg(bill.categoryName, displayLabel(bill.recurrence),
                 bill.nextDue.toString(Qt::ISODate)),
        row);
    details->setStyleSheet(QStringLiteral("color: %1; border: none;").arg(accentColor));

    auto* amount = new QLabel(CurrencyFormatter::format(bill.amount), row);
    amount->setStyleSheet(QStringLiteral("border: none;"));

    auto* payButton = new QPushButton(tr("Mark Paid…"), row);
    const RecurringBill billCopy = bill;
    connect(payButton, &QPushButton::clicked, this, [this, billCopy] {
        AddExpenseDialog dialog(this);
        dialog.setMarkPaidMode(billCopy);
        if (dialog.exec() == QDialog::Accepted)
            emit dataChanged();
    });

    auto* text = new QVBoxLayout;
    text->setContentsMargins(0, 0, 0, 0);
    text->setSpacing(2);
    text->addWidget(name);
    text->addWidget(details);

    auto* layout = new QHBoxLayout(row);
    layout->addLayout(text, 1);
    layout->addWidget(amount);
    layout->addWidget(payButton);
    return row;
}
