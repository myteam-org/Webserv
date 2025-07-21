add_executable(error_page_test error_page_test.cpp)

target_link_libraries(error_page_test
    PRIVATE webserv_lib gtest
)

include(GoogleTest)
gtest_discover_tests(error_page_test)