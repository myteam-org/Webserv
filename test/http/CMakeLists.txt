
add_executable(method_test method_test.cpp)

target_link_libraries(method_test
    PRIVATE webserv_lib gtest
)

include(GoogleTest)
gtest_discover_tests(method_test)

add_subdirectory(request)
