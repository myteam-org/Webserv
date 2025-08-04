include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http/handler
)

add_executable(matcher_test matcher_test.cpp)
target_link_libraries(matcher_test PRIVATE webserv_lib gtest)
gtest_discover_tests(matcher_test)

add_subdirectory(router)
add_subdirectory(file)