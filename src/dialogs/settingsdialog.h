#pragma once

#include <QDialog>

class QComboBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void accept() override;

private:
    QComboBox* m_code = nullptr;
    QComboBox* m_theme = nullptr;
};
