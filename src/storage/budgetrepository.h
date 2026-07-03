#pragma once

#include "core/money.h"
#include "core/result.h"

#include <QHash>

#include <optional>

// Storage access for monthly budgets: one optional overall budget plus
// optional per-category budgets. "No budget set" is a real state, hence
// std::optional rather than a zero sentinel.
namespace BudgetRepository {

[[nodiscard]] std::optional<Money> overallBudget();
[[nodiscard]] QHash<int, Money> categoryBudgets(); // categoryId -> budget

// std::nullopt clears the budget
[[nodiscard]] Result<void> setOverallBudget(std::optional<Money> amount);
[[nodiscard]] Result<void> setCategoryBudget(int categoryId, std::optional<Money> amount);

} // namespace BudgetRepository
