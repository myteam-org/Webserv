include_directories(
    ${CMAKE_SOURCE_DIR}/include/utils
)

add_executable(string_test string_test.cpp)
target_link_libraries(string_test PRIVATE webserv_lib gtest)
gtest_discover_tests(string_test)

add_subdirectory(types)
