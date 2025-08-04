include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http
    ${CMAKE_SOURCE_DIR}/include/http/request
    ${CMAKE_SOURCE_DIR}/include/http/config
)

add_executable(resolver_test resolver_test.cpp)
target_link_libraries(resolver_test PRIVATE webserv_lib gtest)
gtest_discover_tests(resolver_test)

add_subdirectory(read)