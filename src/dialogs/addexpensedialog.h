#pragma once

#include "core/money.h"
#include "storage/billrepository.h"

#include <QDialog>

#include <optional>

class QComboBox;
class QDateEdit;
class QLineEdit;
class QPlainTextEdit;

// One dialog, three jobs: add a new expense, edit an existing one, or
// record a recurring-bill payment (which also advances the due date).
class AddExpenseDialog : public QDialog {
    Q_OBJECT
public:
    explicit AddExpenseDialog(QWidget* parent = nullptr);

    void setEditMode(int expenseId);
    void setMarkPaidMode(const RecurringBill& bill);
    void setPreselectedCategory(int categoryId);

    void accept() override;

private:
    enum class Mode { Add, Edit, MarkPaid };

    void populateCategories(int selectedId = -1);
    [[nodiscard]] std::optional<Money> parsedAmount() const;

    Mode m_mode = Mode::Add;
    int m_expenseId = 0;
    RecurringBill m_bill;

    QComboBox* m_category = nullptr;
    QLineEdit* m_amount = nullptr;
    QDateEdit* m_date = nullptr;
    QLineEdit* m_description = nullptr;
    QPlainTextEdit* m_notes = nullptr;
};
