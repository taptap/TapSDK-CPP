add_executable(tapsdk_test
        main_case.cpp)

target_link_libraries(tapsdk_test PUBLIC tds_core Catch2::Catch2WithMain)
target_include_directories(tapsdk_test PRIVATE . ../source)
add_test(tapsdk_test tapsdk_test --durations yes)

