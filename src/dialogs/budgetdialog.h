#pragma once

#include <QDialog>
#include <QList>
#include <QPair>

class QLineEdit;

// Tools > Monthly Budget: one optional overall budget plus optional
// per-category budgets. An empty field means "no budget".
class BudgetDialog : public QDialog {
    Q_OBJECT
public:
    explicit BudgetDialog(QWidget* parent = nullptr);

    void accept() override;

private:
    QLineEdit* m_overall = nullptr;
    QList<QPair<int, QLineEdit*>> m_categoryEdits; // categoryId -> field
};
