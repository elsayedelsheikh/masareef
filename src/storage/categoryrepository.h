#pragma once

#include "core/result.h"

#include <QList>
#include <QString>

struct Category {
    int id = 0;
    QString name;
    QString color;
};

// Storage access for `categories`, enforcing the business rules: a
// category still referenced by expenses or recurring bills is undeletable
// via remove() (use removeAndReassign() instead), and the last remaining
// category can never be deleted (expenses require one).
namespace CategoryRepository {

[[nodiscard]] QList<Category> all();
[[nodiscard]] Result<void> add(const QString& name, const QString& color);
[[nodiscard]] Result<void> rename(int id, const QString& newName);
[[nodiscard]] Result<void> setColor(int id, const QString& color);
[[nodiscard]] Result<void> remove(int id);
// Moves the category's expenses and recurring bills to targetId, drops its
// budget row, then deletes it — atomically. Rejects id == targetId or a
// missing target.
[[nodiscard]] Result<void> removeAndReassign(int id, int targetId);
[[nodiscard]] bool isInUse(int id);
// First palette slot not already taken by an existing category
[[nodiscard]] QString suggestedColor();

} // namespace CategoryRepository
