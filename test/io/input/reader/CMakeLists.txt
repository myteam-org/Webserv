include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/io/input/read
)

add_executable(fd_test fd_test.cpp)
target_link_libraries(fd_test 
    PRIVATE webserv_lib gtest
)

gtest_discover_tests(fd_test)
