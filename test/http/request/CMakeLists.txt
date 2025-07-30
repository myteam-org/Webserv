include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/types
    ${CMAKE_SOURCE_DIR}/include/http
    ${CMAKE_SOURCE_DIR}/include/http/request
)

# 🔽 test.conf をビルドディレクトリへコピー
configure_file(
    ${CMAKE_SOURCE_DIR}/test/test_config/test.conf
    ${CMAKE_CURRENT_BINARY_DIR}/test_config/test.conf
    COPYONLY
)

# HttpRequest のテスト
add_executable(http_request_test
    http_request_test.cpp
    ${CMAKE_SOURCE_DIR}/src/http/request/http_request.cpp
)
target_link_libraries(http_request_test PRIVATE webserv_lib gtest gtest_main)

# 🔽 定数を渡す
target_compile_definitions(http_request_test PRIVATE
  TEST_CONF_PATH="${CMAKE_CURRENT_BINARY_DIR}/test_config/test.conf"
)

# 🔽 working directory を明示
gtest_discover_tests(http_request_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# RequestReader 関連のテスト
add_executable(request_test
    request_test.cpp
)
target_link_libraries(request_test PRIVATE webserv_lib gtest gtest_main)

# 🔽 これも明示！
gtest_discover_tests(request_test
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# サブディレクトリ
add_subdirectory(read)
add_subdirectory(parse)
