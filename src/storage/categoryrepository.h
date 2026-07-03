#pragma once

#include "core/result.h"

#include <QList>
#include <QString>

enum class CategoryType { System, User };

struct Category {
    int id = 0;
    QString name;
    CategoryType type = CategoryType::User;
    QString color;

    [[nodiscard]] bool isSystem() const { return type == CategoryType::System; }
};

// Storage access for `categories`, enforcing the business rules: system
// categories and categories still referenced by expenses or recurring
// bills are undeletable.
namespace CategoryRepository {

[[nodiscard]] QList<Category> all();
[[nodiscard]] Result<void> add(const QString& name, const QString& color);
[[nodiscard]] Result<void> rename(int id, const QString& newName);
[[nodiscard]] Result<void> setColor(int id, const QString& color);
[[nodiscard]] Result<void> remove(int id);
[[nodiscard]] bool isInUse(int id);
// First palette slot not already taken by an existing category
[[nodiscard]] QString suggestedColor();

} // namespace CategoryRepository
