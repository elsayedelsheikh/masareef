#pragma once

#include <QWidget>

struct RecurringBill;
class QVBoxLayout;

// Bills grouped by urgency: overdue, due within 7 days, due within 30
// days, later. Each row has a Mark Paid button that records the payment
// and advances the due date.
class RemindersWidget : public QWidget {
    Q_OBJECT
public:
    explicit RemindersWidget(QWidget* parent = nullptr);

    void refresh();
    int overdueCount() const { return m_overdueCount; }

signals:
    void dataChanged();

private:
    void addSection(const QString& title, const QString& color,
                    const QList<RecurringBill>& bills);
    QWidget* makeBillRow(const RecurringBill& bill, const QString& accentColor);

    QVBoxLayout* m_sections = nullptr;
    int m_overdueCount = 0;
};
