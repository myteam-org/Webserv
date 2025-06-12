add_executable(ConfigParser_test ConfigParser_test.cpp)

# 依存関係を追加
add_dependencies(ConfigParser_test webserv_lib)

# ライブラリをリンク
target_link_libraries(ConfigParser_test PRIVATE webserv_lib gtest_main gmock_main)

gtest_discover_tests(ConfigParser_test)
