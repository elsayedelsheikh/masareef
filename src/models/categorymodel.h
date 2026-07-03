#pragma once

#include "storage/categoryrepository.h"

#include <QSqlTableModel>

// Editable table model over `categories` used by ManageCategoriesDialog.
// Business rules (undeletable system/in-use categories) live in
// CategoryRepository.
class CategoryModel : public QSqlTableModel {
    Q_OBJECT
public:
    explicit CategoryModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] Category categoryAt(int row) const;
    void reload();
};
