#include "models/categorymodel.h"

#include <QColor>

namespace {
enum TableColumn { TcId = 0, TcName, TcType, TcColor };
} // namespace

CategoryModel::CategoryModel(QObject* parent)
    : QSqlTableModel(parent)
{
    setTable(QStringLiteral("categories"));
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setSort(TcName, Qt::AscendingOrder);
    select();
}

void CategoryModel::reload()
{
    select();
}

QVariant CategoryModel::data(const QModelIndex& index, int role) const
{
    // Color swatch next to the category name
    if (index.column() == TcName && role == Qt::DecorationRole) {
        const QString color =
            QSqlTableModel::data(this->index(index.row(), TcColor)).toString();
        if (!color.isEmpty())
            return QColor(color);
    }
    return QSqlTableModel::data(index, role);
}

Category CategoryModel::categoryAt(int row) const
{
    Category cat;
    cat.id = QSqlTableModel::data(index(row, TcId)).toInt();
    cat.name = QSqlTableModel::data(index(row, TcName)).toString();
    cat.color = QSqlTableModel::data(index(row, TcColor)).toString();
    return cat;
}
