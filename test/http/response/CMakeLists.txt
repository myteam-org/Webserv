include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http
    ${CMAKE_SOURCE_DIR}/include/http/response
)

add_executable(response_test response_test.cpp)
target_link_libraries(response_test PRIVATE webserv_lib gtest)
gtest_discover_tests(response_test)
