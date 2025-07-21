add_executable(method_test method_test.cpp)

target_link_libraries(method_test
    PRIVATE webserv_lib gtest
)

include(GoogleTest)
gtest_discover_tests(method_test)

add_subdirectory(request)
add_subdirectory(handler)
add_subdirectory(response)

add_executable(status_test status_test.cpp)

target_link_libraries(status_test
    PRIVATE webserv_lib gtest
)

include(GoogleTest)
gtest_discover_tests(status_test)

add_executable(virtual_server_test virtual_server_test.cpp)

target_link_libraries(virtual_server_test
    PRIVATE webserv_lib gtest
)

include(GoogleTest)
gtest_discover_tests(virtual_server_test)