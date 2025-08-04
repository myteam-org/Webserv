include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http
    ${CMAKE_SOURCE_DIR}/include/http/request
    ${CMAKE_SOURCE_DIR}/include/http/request/parse
    ${CMAKE_SOURCE_DIR}/include/http/config
)

add_executable(request_parse_test
    request_parse_test.cpp
    ${CMAKE_SOURCE_DIR}/src/http/request/parse/request_parser.cpp
)
target_link_libraries(request_parse_test PRIVATE webserv_lib gtest gtest_main)
gtest_discover_tests(request_parse_test)
