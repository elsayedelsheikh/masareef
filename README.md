# Masareef

A desktop expense tracker built with Qt 6 Widgets and modern C++23. Track
daily expenses by category, manage recurring bills with due-date reminders,
set monthly budgets, and review spending in charts — all stored locally in
SQLite with automatic startup backups.

The codebase doubles as a modern-C++ reference: `std::expected`-based error
handling, strong value types, a repository layer that owns all SQL, and
designated initializers throughout. See [Architecture](#architecture).

## Features

- Expense list with date-range, category and text filters plus a live total
- Recurring bills (monthly / quarterly / yearly) with overdue and due-soon
  reminders; "Mark Paid" records the expense and advances the due date in
  one transaction
- Monthly budgets — one overall budget plus optional per-category budgets
- Dashboard with month-to-date stat cards and spending by category
- Bar chart of the last 12 months and a per-category pie chart
- Light / dark / follow-system theme
- Manual backup & restore, plus automatic startup backups (newest 10 kept)

## Requirements

- CMake ≥ 3.16
- A C++23 compiler (GCC 13+ or equivalent — `std::expected` is required)
- Qt 6 with the **Widgets**, **Sql**, **Charts** and **Test** modules
  (developed against Qt 6.9)

On Ubuntu/Debian, a distribution Qt works if it ships Qt Charts
(`qt6-base-dev qt6-charts-dev`); otherwise install Qt via the
[online installer](https://www.qt.io/download-qt-installer) or
[aqtinstall](https://github.com/miurahr/aqtinstall) and point
`CMAKE_PREFIX_PATH` at it.

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=$HOME/Qt/6.9.0/gcc_64   # omit if Qt comes from the system
cmake --build build -j$(nproc)
./build/masareef
```

`CMAKE_PREFIX_PATH` must point at the Qt kit directory (the one containing
`lib/cmake/Qt6`).

### Running the tests

Tests build by default (`-DMASAREEF_BUILD_TESTS=OFF` to skip). They use
`QStandardPaths` test mode, so they never touch your real database.

```sh
cd build && ctest --output-on-failure
```

### Windows

A PowerShell script produces a portable zip (it downloads its own Qt and
MinGW toolchain on first run) — see [packaging/README.md](packaging/README.md).

## Architecture

```
src/
├── core/       Qt-free value types: Money, Result<T> (std::expected), Recurrence
├── db/         DatabaseManager — SQLite connection, migrations, seeding
├── storage/    Repositories (Expense, Category, Bill, Budget) — all SQL lives here
├── models/     Thin QAbstractItemModel adapters over the repositories
├── dialogs/    Add/edit expense, categories, recurring bills, budget, settings
├── widgets/    Dashboard, expense list, charts, reminders tabs
├── utils/      Theme/palette, currency formatting, config, backups
└── main.cpp / mainwindow.cpp
tests/          One QTest suite per layer, run via ctest
```

Conventions worth noting if you're reading the code as a reference:

- **Errors are values.** Anything that can fail returns
  `Result<T> = std::expected<T, Error>`; there are no bool + out-param
  signatures. Callers use `if (auto res = ...; !res)` and surface
  `res.error().message`.
- **Amounts are `Money`**, an integer minor-units value type — an amount
  can't be confused with an id, and money×money doesn't compile.
- **"Not set" is `std::optional`**, not a zero sentinel (budgets,
  bill links).
- **Repositories are free-function namespaces**; the GUI never writes SQL,
  and models only adapt query results for views.

The app and tests share the `masareef_core` static library, which contains
everything except the GUI.

## Data location

The SQLite database lives under the per-user app-data directory
(`~/.local/share/Masareef` on Linux, `%APPDATA%\Masareef` on Windows) with
automatic backups in a `backups/` subfolder next to it.
