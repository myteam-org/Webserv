include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http
    ${CMAKE_SOURCE_DIR}/include/http/request
)

# HttpRequest のテスト
add_executable(http_request_test
    http_request_test.cpp
    ${CMAKE_SOURCE_DIR}/src/http/request/http_request.cpp
)
target_link_libraries(http_request_test PRIVATE webserv_lib gtest gtest_main)
gtest_discover_tests(http_request_test)

# RequestReader 関連のテスト（例: リクエストパースなど）
add_executable(request_test
    request_test.cpp
)
target_link_libraries(request_test PRIVATE webserv_lib gtest gtest_main)
gtest_discover_tests(request_test)

# test/http/request/CMakeLists.txt に追加
configure_file(
    ${CMAKE_SOURCE_DIR}/test/test_config/test.conf
    ${CMAKE_CURRENT_BINARY_DIR}/test_config/test.conf
    COPYONLY
)

add_subdirectory(read)
add_subdirectory(parse)
