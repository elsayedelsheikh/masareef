#pragma once

#include <QDialog>

class QComboBox;
class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void accept() override;

private:
    QComboBox* m_code = nullptr;
    QLineEdit* m_symbol = nullptr;
};
