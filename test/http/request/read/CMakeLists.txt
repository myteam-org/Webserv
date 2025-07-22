cmake_minimum_required(VERSION 3.14)
project(getline_test CXX)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/io/input/read
    ${CMAKE_SOURCE_DIR}/include/io/input/reader
    ${CMAKE_SOURCE_DIR}/include/http/request/read
    ${CMAKE_SOURCE_DIR}/include/config
    ${CMAKE_SOURCE_DIR}/include/http/request
)

add_executable(getline_test utils_test.cpp)
target_link_libraries(getline_test
    PRIVATE webserv_lib gtest
)

add_executable(reader_test reader_test.cpp)
target_link_libraries(reader_test
    PRIVATE webserv_lib gtest
)

add_executable(line_test line_test.cpp)
target_link_libraries(line_test
    PRIVATE webserv_lib gtest
)

add_executable(context_test context_test.cpp)
target_link_libraries(context_test
    PRIVATE webserv_lib gtest
)

add_executable(header_test header_test.cpp)
target_link_libraries(header_test
    PRIVATE webserv_lib gtest
)

add_executable(body_test body_test.cpp)
target_link_libraries(body_test
    PRIVATE webserv_lib gtest
)

add_executable(body_length_test body_length_test.cpp)
target_link_libraries(body_length_test
    PRIVATE webserv_lib gtest)

include(GoogleTest)
gtest_discover_tests(getline_test)
gtest_discover_tests(reader_test)
gtest_discover_tests(line_test)
gtest_discover_tests(context_test)
gtest_discover_tests(header_test)
gtest_discover_tests(body_test) 
gtest_discover_tests(body_length_test)
