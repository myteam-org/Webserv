include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http
    ${CMAKE_SOURCE_DIR}/include/http/request
)

add_executable(request_test request_test.cpp)
target_link_libraries(request_test PRIVATE webserv_lib gtest)
gtest_discover_tests(request_test)

add_subdirectory(read)