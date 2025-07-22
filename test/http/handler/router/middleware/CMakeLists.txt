add_executable(error_page_test error_page_test.cpp)

target_link_libraries(error_page_test
    PRIVATE webserv_lib gtest
)

include(GoogleTest)
gtest_discover_tests(error_page_test)

add_executable(logger_test logger_test.cpp)

target_link_libraries(logger_test PRIVATE webserv_lib gtest gtest_main)

gtest_discover_tests(logger_test)
