find_package(Qt6 REQUIRED COMPONENTS Test)

set(MASAREEF_TESTS
    recurrence
    appconfig
    currencyformatter
    databasemanager
    categoryrepository
    expenserepository
    expensemodel
    billrepository
    budgetrepository
    backupmanager
)

foreach(name IN LISTS MASAREEF_TESTS)
    add_executable(tst_${name} tst_${name}.cpp)
    target_link_libraries(tst_${name} PRIVATE masareef_core Qt6::Test)
    add_test(NAME tst_${name} COMMAND tst_${name})
endforeach()

if(TARGET masareef_quick)
    add_subdirectory(mobile)
endif()
