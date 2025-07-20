include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http/handler/router
    ${CMAKE_SOURCE_DIR}/include/http/handler
    ${CMAKE_SOURCE_DIR}/include/http/request
    ${CMAKE_SOURCE_DIR}/include/http/response
    ${CMAKE_SOURCE_DIR}/include/action
)

add_executable(registry_test registry_test.cpp)
target_link_libraries(registry_test PRIVATE webserv_lib gtest)
gtest_discover_tests(registry_test)

add_executable(router_test router_test.cpp)
target_link_libraries(router_test PRIVATE webserv_lib gtest)
gtest_discover_tests(router_test)
add_subdirectory(middleware)
