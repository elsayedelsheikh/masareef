#pragma once

#include <QDialog>

class QPushButton;
class QTableWidget;

class RecurringBillsDialog : public QDialog {
    Q_OBJECT
public:
    explicit RecurringBillsDialog(QWidget* parent = nullptr);

private slots:
    void addBill();
    void editBill();
    void deleteBill();
    void toggleActive();
    void updateButtonStates();

private:
    void reload();
    int selectedBillId() const;

    QTableWidget* m_table = nullptr;
    QPushButton* m_editButton = nullptr;
    QPushButton* m_deleteButton = nullptr;
    QPushButton* m_toggleButton = nullptr;
};
