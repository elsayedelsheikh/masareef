#include "dialogs/managecategoriesdialog.h"

#include "models/categorymodel.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ManageCategoriesDialog::ManageCategoriesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Manage Categories"));
    setMinimumSize(360, 320);

    m_model = new CategoryModel(this);
    m_view = new QListView(this);
    m_view->setModel(m_model);
    m_view->setModelColumn(1); // name column, with the color swatch decoration
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* addButton = new QPushButton(tr("Add…"), this);
    m_renameButton = new QPushButton(tr("Rename…"), this);
    m_colorButton = new QPushButton(tr("Color…"), this);
    m_deleteButton = new QPushButton(tr("Delete"), this);

    connect(addButton, &QPushButton::clicked, this, &ManageCategoriesDialog::addCategory);
    connect(m_renameButton, &QPushButton::clicked, this,
            &ManageCategoriesDialog::renameCategory);
    connect(m_colorButton, &QPushButton::clicked, this, &ManageCategoriesDialog::changeColor);
    connect(m_deleteButton, &QPushButton::clicked, this,
            &ManageCategoriesDialog::deleteCategory);
    connect(m_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &ManageCategoriesDialog::updateButtonStates);

    auto* buttonColumn = new QVBoxLayout;
    buttonColumn->addWidget(addButton);
    buttonColumn->addWidget(m_renameButton);
    buttonColumn->addWidget(m_colorButton);
    buttonColumn->addWidget(m_deleteButton);
    buttonColumn->addStretch();

    auto* listRow = new QHBoxLayout;
    listRow->addWidget(m_view, 1);
    listRow->addLayout(buttonColumn);

    auto* closeBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(closeBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(listRow, 1);
    layout->addWidget(closeBox);

    updateButtonStates();
}

int ManageCategoriesDialog::selectedRow() const
{
    const QModelIndex current = m_view->currentIndex();
    return current.isValid() ? current.row() : -1;
}

void ManageCategoriesDialog::updateButtonStates()
{
    const int row = selectedRow();
    const bool hasSelection = row >= 0;
    const bool isSystem = hasSelection && m_model->categoryAt(row).isSystem();
    m_renameButton->setEnabled(hasSelection && !isSystem);
    m_deleteButton->setEnabled(hasSelection && !isSystem);
    m_colorButton->setEnabled(hasSelection);
}

void ManageCategoriesDialog::addCategory()
{
    const QString name = QInputDialog::getText(this, tr("Add Category"),
                                               tr("Category name:"));
    if (name.trimmed().isEmpty())
        return;

    QString error;
    if (!CategoryModel::addCategory(name, CategoryModel::suggestedColor(), &error)) {
        QMessageBox::warning(this, tr("Add Category"), error);
        return;
    }
    m_model->reload();
}

void ManageCategoriesDialog::renameCategory()
{
    const int row = selectedRow();
    if (row < 0)
        return;
    const Category cat = m_model->categoryAt(row);
    const QString name = QInputDialog::getText(this, tr("Rename Category"),
                                               tr("New name:"), QLineEdit::Normal, cat.name);
    if (name.trimmed().isEmpty() || name == cat.name)
        return;

    QString error;
    if (!CategoryModel::renameCategory(cat.id, name, &error)) {
        QMessageBox::warning(this, tr("Rename Category"), error);
        return;
    }
    m_model->reload();
}

void ManageCategoriesDialog::changeColor()
{
    const int row = selectedRow();
    if (row < 0)
        return;
    const Category cat = m_model->categoryAt(row);
    const QColor color = QColorDialog::getColor(QColor(cat.color), this,
                                                tr("Color for %1").arg(cat.name));
    if (!color.isValid())
        return;

    QString error;
    if (!CategoryModel::setColor(cat.id, color.name(), &error)) {
        QMessageBox::warning(this, tr("Category Color"), error);
        return;
    }
    m_model->reload();
}

void ManageCategoriesDialog::deleteCategory()
{
    const int row = selectedRow();
    if (row < 0)
        return;
    const Category cat = m_model->categoryAt(row);
    if (QMessageBox::question(this, tr("Delete Category"),
                              tr("Delete the category \"%1\"?").arg(cat.name))
        != QMessageBox::Yes)
        return;

    QString error;
    if (!CategoryModel::removeCategory(cat.id, &error)) {
        QMessageBox::warning(this, tr("Delete Category"), error);
        return;
    }
    m_model->reload();
}
