# Host-run tests for the QML-facing backend (only when masareef_quick is built)
set(MASAREEF_MOBILE_TESTS
    expenselistmodel
    categorylistmodel
    expensecontroller
    dashboardviewmodel
    budgetviewmodel
    appbackend
    themecontroller
)

find_package(Qt6 REQUIRED COMPONENTS QuickTest)

foreach(name IN LISTS MASAREEF_MOBILE_TESTS)
    add_executable(tst_${name} tst_${name}.cpp)
    target_link_libraries(tst_${name} PRIVATE masareef_quick Qt6::Test)
    target_include_directories(tst_${name} PRIVATE ..)
    add_test(NAME tst_${name} COMMAND tst_${name})
    # QTEST_MAIN tests spin up a QGuiApplication; keep them display-free
    set_tests_properties(tst_${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()

# Qt Quick Test suite driving the QML components/sheets/screens
add_executable(tst_qml qml/tst_qml_main.cpp)
target_link_libraries(tst_qml PRIVATE
    masareef_quick
    masareef_quickplugin
    Qt6::QuickTest
)
target_include_directories(tst_qml PRIVATE .)
add_test(NAME tst_qml
    COMMAND tst_qml -input ${CMAKE_CURRENT_SOURCE_DIR}/qml)
set_tests_properties(tst_qml PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_CONTROLS_STYLE=Material")
