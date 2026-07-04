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
    m_renameButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection && m_model->rowCount() > 1);
    m_colorButton->setEnabled(hasSelection);
}

void ManageCategoriesDialog::addCategory()
{
    const QString name = QInputDialog::getText(this, tr("Add Category"),
                                               tr("Category name:"));
    if (name.trimmed().isEmpty())
        return;

    if (auto res = CategoryRepository::add(name, CategoryRepository::suggestedColor());
        !res) {
        QMessageBox::warning(this, tr("Add Category"), res.error().message);
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

    if (auto res = CategoryRepository::rename(cat.id, name); !res) {
        QMessageBox::warning(this, tr("Rename Category"), res.error().message);
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

    if (auto res = CategoryRepository::setColor(cat.id, color.name()); !res) {
        QMessageBox::warning(this, tr("Category Color"), res.error().message);
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

    if (!CategoryRepository::isInUse(cat.id)) {
        if (QMessageBox::question(this, tr("Delete Category"),
                                  tr("Delete the category \"%1\"?").arg(cat.name))
            != QMessageBox::Yes)
            return;
        if (auto res = CategoryRepository::remove(cat.id); !res)
            QMessageBox::warning(this, tr("Delete Category"), res.error().message);
        m_model->reload();
        return;
    }

    QStringList otherNames;
    QList<int> otherIds;
    for (const Category& other : CategoryRepository::all()) {
        if (other.id == cat.id)
            continue;
        otherNames << other.name;
        otherIds << other.id;
    }

    bool accepted = false;
    const QString chosen = QInputDialog::getItem(
        this, tr("Delete Category"),
        tr("\"%1\" is used by existing expenses or recurring bills.\n"
           "Move them to:").arg(cat.name),
        otherNames, 0, false, &accepted);
    if (!accepted)
        return;

    const int targetId = otherIds.at(otherNames.indexOf(chosen));
    if (auto res = CategoryRepository::removeAndReassign(cat.id, targetId); !res)
        QMessageBox::warning(this, tr("Delete Category"), res.error().message);
    m_model->reload();
}
