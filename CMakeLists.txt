cmake_minimum_required(VERSION 3.16)
project(masareef VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Charts)

# Non-GUI core (storage, models, utils) — shared by the app and the tests
add_library(masareef_core STATIC
    src/db/databasemanager.cpp
    src/db/databasemanager.h
    src/models/expensemodel.cpp
    src/models/expensemodel.h
    src/models/categorymodel.cpp
    src/models/categorymodel.h
    src/models/recurringbillmodel.cpp
    src/models/recurringbillmodel.h
    src/utils/appconfig.cpp
    src/utils/appconfig.h
    src/utils/currencyformatter.cpp
    src/utils/currencyformatter.h
    src/utils/backupmanager.cpp
    src/utils/backupmanager.h
    src/utils/palette.h
)
target_include_directories(masareef_core PUBLIC src)
target_link_libraries(masareef_core PUBLIC Qt6::Core Qt6::Gui Qt6::Sql)

add_executable(masareef WIN32
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/dialogs/addexpensedialog.cpp
    src/dialogs/addexpensedialog.h
    src/dialogs/managecategoriesdialog.cpp
    src/dialogs/managecategoriesdialog.h
    src/dialogs/recurringbillsdialog.cpp
    src/dialogs/recurringbillsdialog.h
    src/dialogs/settingsdialog.cpp
    src/dialogs/settingsdialog.h
    src/widgets/dashboardwidget.cpp
    src/widgets/dashboardwidget.h
    src/widgets/expenselistwidget.cpp
    src/widgets/expenselistwidget.h
    src/widgets/chartswidget.cpp
    src/widgets/chartswidget.h
    src/widgets/reminderswidget.cpp
    src/widgets/reminderswidget.h
)
target_link_libraries(masareef PRIVATE masareef_core Qt6::Widgets Qt6::Charts)

enable_testing()
add_subdirectory(tests)
