# QML backend + mobile app. Built only under MASAREEF_BUILD_QML or ANDROID,
# so the system-Qt desktop build never sees these targets.
find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Svg LinguistTools)

qt_standard_project_setup(REQUIRES 6.8)

# Backend C++ + QML module, shared by the app and the host-run tests
qt_add_library(masareef_quick STATIC)
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(masareef_quick
    URI Masareef
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/components/AmountField.qml
        qml/components/BudgetGauge.qml
        qml/components/CategoryPicker.qml
        qml/components/DateField.qml
        qml/components/EmptyState.qml
        qml/components/ExpenseDelegate.qml
        qml/components/ExpenseForm.qml
        qml/components/NavBar.qml
        qml/components/Snackbar.qml
        qml/components/StatCard.qml
        qml/sheets/AddExpenseSheet.qml
        qml/sheets/EditExpenseSheet.qml
        qml/screens/BudgetsScreen.qml
        qml/screens/CategoryManagerScreen.qml
        qml/screens/ExpensesScreen.qml
        qml/screens/HomeScreen.qml
        qml/screens/SettingsScreen.qml
    RESOURCES
        qml/icons/calendar.svg
        qml/icons/check.svg
        qml/icons/chevron-left.svg
        qml/icons/chevron-right.svg
        qml/icons/close.svg
        qml/icons/donut.svg
        qml/icons/gear.svg
        qml/icons/home.svg
        qml/icons/pencil.svg
        qml/icons/plus.svg
        qml/icons/receipt.svg
        qml/icons/search.svg
        qml/icons/tag.svg
        qml/icons/trash.svg
        qml/icons/wallet.svg
    SOURCES
        backend/appbackend.cpp
        backend/appbackend.h
        backend/budgetviewmodel.cpp
        backend/budgetviewmodel.h
        backend/categorylistmodel.cpp
        backend/categorylistmodel.h
        backend/dashboardviewmodel.cpp
        backend/dashboardviewmodel.h
        backend/expensecontroller.cpp
        backend/expensecontroller.h
        backend/expenselistmodel.cpp
        backend/expenselistmodel.h
        backend/themecontroller.cpp
        backend/themecontroller.h
)
# "." for the app/tests ("backend/foo.h"); "backend" because the generated
# QML type registration includes the headers by bare name.
target_include_directories(masareef_quick PUBLIC . PRIVATE backend)
target_link_libraries(masareef_quick PUBLIC masareef_core Qt6::Quick)

# Arabic catalog is embedded under :/i18n and installed by AppBackend
qt_add_translations(masareef_quick
    SOURCE_TARGETS masareef_quick masareef_core
    TS_FILES i18n/masareef_ar.ts
    RESOURCE_PREFIX "/i18n"
)

qt_add_executable(masareef_mobile main.cpp)
qt_add_resources(masareef_mobile "controls_conf"
    PREFIX "/"
    FILES qtquickcontrols2.conf
)
target_compile_definitions(masareef_mobile
    PRIVATE MASAREEF_VERSION="${PROJECT_VERSION}")
target_link_libraries(masareef_mobile PRIVATE
    masareef_quick
    masareef_quickplugin
    Qt6::QuickControls2
    # Icons are stroke SVGs; make sure the svg image plugin ships in the APK
    Qt6::Svg
)

if(ANDROID)
    set_target_properties(masareef_mobile PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.masareef.app"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_VERSION_NAME "${PROJECT_VERSION}"
        QT_ANDROID_VERSION_CODE 1
        QT_ANDROID_TARGET_SDK_VERSION 34
    )
endif()
