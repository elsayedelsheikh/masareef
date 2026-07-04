#include "backend/categorylistmodel.h"

#include "utils/palette.h"

CategoryListModel::CategoryListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    reload();
}

int CategoryListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : int(m_rows.size());
}

QVariant CategoryListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};

    const Row& row = m_rows.at(index.row());
    switch (role) {
    case CategoryIdRole:
        return row.category.id;
    case NameRole:
        return row.category.name;
    case ColorRole:
        return Palette::series(row.category.color);
    case InUseRole:
        return row.inUse;
    default:
        return {};
    }
}

QHash<int, QByteArray> CategoryListModel::roleNames() const
{
    return {
        { CategoryIdRole, "categoryId" },
        { NameRole, "name" },
        { ColorRole, "color" },
        { InUseRole, "inUse" },
    };
}

void CategoryListModel::refresh()
{
    reload();
}

bool CategoryListModel::add(const QString& name, const QString& color)
{
    return applyResult(CategoryRepository::add(name, color));
}

bool CategoryListModel::rename(int id, const QString& newName)
{
    return applyResult(CategoryRepository::rename(id, newName));
}

bool CategoryListModel::setColor(int id, const QString& color)
{
    const Result<void> result = CategoryRepository::setColor(id, color);
    const QString error = result ? QString() : result.error().message;
    if (error != m_lastError) {
        m_lastError = error;
        emit lastErrorChanged();
    }
    if (!result)
        return false;

    // A color change never reorders the list — update in place so QML
    // delegates keep their position instead of resetting the whole view.
    for (int row = 0; row < m_rows.size(); ++row) {
        if (m_rows.at(row).category.id == id) {
            m_rows[row].category.color = color;
            const QModelIndex index = this->index(row, 0);
            emit dataChanged(index, index, { ColorRole });
            break;
        }
    }
    return true;
}

bool CategoryListModel::remove(int id)
{
    return applyResult(CategoryRepository::remove(id));
}

bool CategoryListModel::removeAndReassign(int id, int targetId)
{
    return applyResult(CategoryRepository::removeAndReassign(id, targetId));
}

QString CategoryListModel::suggestedColor() const
{
    return CategoryRepository::suggestedColor();
}

QStringList CategoryListModel::paletteColors() const
{
    QStringList colors;
    colors.reserve(Palette::kCategoricalCount);
    for (int i = 0; i < Palette::kCategoricalCount; ++i)
        colors.append(QString::fromLatin1(Palette::kCategorical[i]));
    return colors;
}

void CategoryListModel::reload()
{
    QList<Row> rows;
    for (Category& category : CategoryRepository::all()) {
        const bool inUse = CategoryRepository::isInUse(category.id);
        rows.append({ std::move(category), inUse });
    }

    const int oldCount = int(m_rows.size());
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
    if (oldCount != m_rows.size())
        emit countChanged();
}

// Shared tail of every mutation: record the error (or clear it), reload on
// success so the model always mirrors the repository.
bool CategoryListModel::applyResult(const Result<void>& result)
{
    const QString error = result ? QString() : result.error().message;
    if (error != m_lastError) {
        m_lastError = error;
        emit lastErrorChanged();
    }
    if (!result)
        return false;
    reload();
    return true;
}
