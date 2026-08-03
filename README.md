# Masareef

An expense tracker built with Qt 6 and modern C++23 — a Widgets desktop app
(Linux/Windows) plus a Qt Quick **Android app**. Track daily expenses by
category, manage recurring bills with due-date reminders, set monthly
budgets, and review spending in charts — all stored locally in SQLite with
automatic startup backups.

The codebase doubles as a modern-C++ reference: `std::expected`-based error
handling, strong value types, a repository layer that owns all SQL, and
designated initializers throughout. See [Architecture](#architecture).

## Features

- Expense list with date-range, category and text filters plus a live total
- Recurring bills (monthly / quarterly / yearly) with overdue and due-soon
  reminders; "Mark Paid" records the expense and advances the due date in
  one transaction. Bills can be paused and resumed, and the mobile app
  totals what they cost per month (quarterly ÷ 3, yearly ÷ 12)
- Price book (mobile) — a catalog of what things normally cost, per unit,
  with the price history behind it, so a row can show which way a price
  last moved and by how much. Any entry can be logged straight as an expense
- Monthly budgets — one overall budget plus optional per-category budgets
- Dashboard with month-to-date stat cards and spending by category
- Bar chart of the last 12 months and a per-category pie chart
- Light / dark / follow-system theme
- Manual backup & restore, plus automatic startup backups (newest 10 kept)

## Requirements

- CMake ≥ 3.16
- A C++23 compiler (GCC 13+ or equivalent — `std::expected` is required)
- Qt **≥ 6.5** with the **Widgets**, **Sql**, **Charts** and **Test**
  modules (developed against Qt 6.9). Older Qt is rejected at configure
  time: moc before 6.5 cannot parse the C++23 standard headers
  (`<expected>`, `<concepts>`) this codebase uses.
- The QML app (`-DMASAREEF_BUILD_QML=ON`) additionally needs Qt **≥ 6.8**
  with **Quick**, **QuickControls2**, **Svg**, **QuickTest** and the
  Linguist tools.

### Fedora

Fedora's own Qt is new enough for everything, including the QML app — no
`CMAKE_PREFIX_PATH` and no Qt installer needed. One command covers the
desktop app, the QML app and the full test suite:

```sh
sudo dnf install cmake ninja-build gcc-c++ \
     qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel \
     qt6-qttools-devel qt6-qtcharts-devel
```

| Package | Needed for |
|---|---|
| `qt6-qtbase-devel` | Core, Gui, Widgets, Sql, Test — the desktop app |
| `qt6-qtcharts-devel` | the desktop charts tab |
| `qt6-qtdeclarative-devel` | Quick, QuickControls2, QuickTest — the QML app |
| `qt6-qtsvg-devel` | the QML app's stroke-SVG icons |
| `qt6-qttools-devel` | `lrelease`, which compiles the Arabic catalog |

Then build and test both configurations:

```sh
# Desktop (Widgets)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure
./build/masareef

# Desktop + QML app + the mobile test suites
cmake -S . -B build-qml -G Ninja -DCMAKE_BUILD_TYPE=Debug -DMASAREEF_BUILD_QML=ON
cmake --build build-qml -j$(nproc)
ctest --test-dir build-qml --output-on-failure
./build-qml/src/mobile/masareef_mobile
```

Note the explicit `-DMASAREEF_BUILD_QML=ON` rather than the `host-qml`
preset: that preset hardcodes `/opt/Qt/6.11.0/gcc_64` and `g++-13`, neither
of which exists on a stock Fedora. Verified on Fedora 43 (Qt 6.10.3,
GCC 15). Android APKs still need a Qt for Android kit — see
[Android / QML app](#android--qml-app).

If you would rather not install Qt on the host, the same build runs in a
container:

```sh
podman run --rm -it -v "$PWD":/src:Z -w /src registry.fedoraproject.org/fedora:43 \
  sh -c 'dnf install -y cmake ninja-build gcc-c++ qt6-qtbase-devel \
           qt6-qtdeclarative-devel qt6-qtsvg-devel qt6-qttools-devel \
           qt6-qtcharts-devel &&
         cmake -S /src -B /tmp/b -G Ninja -DMASAREEF_BUILD_QML=ON &&
         cmake --build /tmp/b -j$(nproc) &&
         QT_QPA_PLATFORM=offscreen ctest --test-dir /tmp/b --output-on-failure'
```

### Ubuntu / Debian

A distribution Qt works if it is new enough and ships Qt Charts
(`qt6-base-dev qt6-charts-dev`). **Ubuntu 24.04 LTS ships Qt 6.4.2, which is
too old** — Ubuntu 24.10 and later (Qt 6.6+) are fine. On 24.04, install Qt
via the [online installer](https://www.qt.io/download-qt-installer) or
[aqtinstall](https://github.com/miurahr/aqtinstall) and point
`CMAKE_PREFIX_PATH` at it (or use the `host-qml` preset, which does this
for a `/opt/Qt` install — see below).

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

## Android / QML app

`src/mobile/` contains a Qt Quick (Material style) app sharing the same
`masareef_core` library and database schema: expenses, categories,
dashboard, budgets, bills and the price book, with English + Arabic (full
RTL) and light/dark themes. Application ID: `com.masareef.app`.

Interaction conventions worth knowing before reading the QML: every list row
is tap-to-edit and long-press-for-actions (an `ActionSheet`, because a
`Menu`'s popup sits outside the mirrored item tree and would stay
left-aligned in Arabic). The add/edit sheets never dismiss themselves — no
drag-to-close, no close-on-tap-outside — and they keep their draft when
closed, so an interruption does not throw away a half-typed entry.

Requirements: Qt ≥ 6.8 for Android (arm64-v8a and/or x86_64 kits) **plus the
matching host kit** (`gcc_64`), Android SDK with NDK r27, JDK 17+.

`CMakePresets.json` ships four presets; adjust the Qt/SDK/NDK paths there
(or shadow them in a `CMakeUserPresets.json`) to match your machine:

| Preset | Purpose |
|---|---|
| `desktop` | Widgets app against system Qt (current behavior) |
| `host-qml` | Desktop + QML app + **all** tests on the host Qt kit |
| `android-arm64` | APK for real devices |
| `android-x86_64` | APK for the emulator |

If the system Qt is older than 6.5 (e.g. Ubuntu 24.04 LTS), the `desktop`
preset still works — just point it at an installer kit:

```sh
cmake --preset desktop -DCMAKE_PREFIX_PATH=/opt/Qt/6.11.0/gcc_64
cmake --build --preset desktop -j$(nproc)
```

```sh
# Develop/test on the host (the QML app runs in a desktop window)
cmake --preset host-qml && cmake --build build-qml -j$(nproc)
ctest --test-dir build-qml --output-on-failure
./build-qml/src/mobile/masareef_mobile

# Android APK
cmake --preset android-arm64 && cmake --build build-android -j$(nproc)
# → build-android/src/mobile/android-build/build/outputs/apk/debug/

# Emulator install
adb install -r build-android-x86_64/src/mobile/android-build/build/outputs/apk/debug/android-build-debug.apk
```

The mobile backend (list models, controllers, view models) lives in
`src/mobile/backend/` and is covered by `tests/mobile/` — QTest suites plus
a Qt Quick Test suite for the QML screens.

### Arabic

Arabic is a first-class UI language, not a translation layer bolted on:

- Strings live in `src/mobile/i18n/masareef_ar.ts`. After changing UI text,
  run `cmake --build build-qml --target update_translations` to merge the
  new sources in, then fill in the empty entries. `lrelease` runs as part of
  the normal build and prints how many messages it accepted — if that count
  is short, a message was malformed rather than merely untranslated.
- **Arabic takes six plural forms**, in Qt's order: zero, one, two, 3–10,
  11–99, 100+. A `<message numerus="yes">` with the wrong number of
  `<numerusform>` entries is the usual reason a string comes out
  untranslated, and it is the first thing to check.
- **Dates and numbers go through `LocaleFormat`, never `QDate::toString`
  or QML's `toLocaleDateString`.** Those format through the *system* locale
  rather than the language the UI is running in, and on an Arabic device
  they emit Arabic-Indic digits. That is not only cosmetic: a locale-formatted
  date used as a lookup key silently matches nothing. `LocaleFormat`
  localizes month and day names while keeping numerals Western, so a row
  never mixes "١٥ يناير" with "150.00 EGP".
- Amounts are parsed by `CurrencyFormatter::parse`, which folds Arabic-Indic
  and Extended Arabic-Indic digits, the Arabic decimal separator and the
  bidi control characters IMEs wrap numbers in back to ASCII. Group
  separators are dropped rather than translated — mapping ٬ to `,` would
  turn one thousand into one, since this parser reads `,` as a decimal point.
- Popups need `LayoutMirroring` attached to their *content root*: a `Popup`
  is not an `Item`, so it does not inherit the window's mirroring. Anything
  that forgets this stays left-aligned in an otherwise right-to-left UI.

Emulator note: if the app freezes (ANR) on an emulator, start it with
`-feature -ClipboardSharing` — the emulator's clipboard proxy can block
Qt's clipboard queries indefinitely. Real devices are unaffected.

## Architecture

```
src/
├── core/       Qt-free value types: Money, Result<T> (std::expected), Recurrence
├── db/         DatabaseManager — SQLite connection, migrations, seeding
├── storage/    Repositories (Expense, Category, Bill, Budget, PriceItem) — all SQL lives here
├── models/     Thin QAbstractItemModel adapters over the repositories
├── dialogs/    Add/edit expense, categories, recurring bills, budget, settings  (desktop)
├── widgets/    Dashboard, expense list, charts, reminders tabs                  (desktop)
├── utils/      Theme/palette, currency formatting, locale formatting, config, backups
├── mobile/
│   ├── backend/    List models, controllers and view models exposed to QML
│   ├── qml/
│   │   ├── components/  Reusable pieces (sheets, delegates, fields, icons)
│   │   ├── screens/     One file per tab, plus Reports and Category manager
│   │   └── sheets/      Add/edit bottom sheets, one pair per entity
│   └── i18n/       masareef_ar.ts — the Arabic catalog
└── main.cpp / mainwindow.cpp
tests/          One QTest suite per layer, run via ctest
└── mobile/qml/ Qt Quick Test suites driving the QML screens
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
- **The schema moves forward only, one numbered step at a time.**
  `DatabaseManager::migrate` runs every step the file has not seen yet,
  in order, inside a single transaction, so a half-migrated database can
  never be committed. A step has to be correct once on a database that has
  not seen it; it is not required to be re-runnable. Adding one means a new
  `kVnStatements` list and bumping `kSchemaVersion` — never editing an
  existing step, which has already run on real data.

The app and tests share the `masareef_core` static library, which contains
everything except the GUI.

## Data location

The SQLite database lives under the per-user app-data directory
(`~/.local/share/Masareef` on Linux, `%APPDATA%\Masareef` on Windows) with
automatic backups in a `backups/` subfolder next to it.
