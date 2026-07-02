#pragma once

#include <QDialog>

class CategoryModel;
class QListView;
class QPushButton;

class ManageCategoriesDialog : public QDialog {
    Q_OBJECT
public:
    explicit ManageCategoriesDialog(QWidget* parent = nullptr);

private slots:
    void addCategory();
    void renameCategory();
    void changeColor();
    void deleteCategory();
    void updateButtonStates();

private:
    int selectedRow() const;

    CategoryModel* m_model = nullptr;
    QListView* m_view = nullptr;
    QPushButton* m_renameButton = nullptr;
    QPushButton* m_colorButton = nullptr;
    QPushButton* m_deleteButton = nullptr;
};
