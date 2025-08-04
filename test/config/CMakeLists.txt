add_executable(parser_test parser_test.cpp)

# 依存関係を追加
add_dependencies(parser_test webserv_lib)

# ライブラリをリンク
target_link_libraries(parser_test PRIVATE webserv_lib gtest_main gmock_main)

gtest_discover_tests(parser_test)

add_subdirectory(context)