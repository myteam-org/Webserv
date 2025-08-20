include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/io/input/writer
)

add_executable(write_buffer_test buffer_test.cpp)
target_link_libraries(write_buffer_test 
    PRIVATE webserv_lib gtest
)

gtest_discover_tests(write_buffer_test)
