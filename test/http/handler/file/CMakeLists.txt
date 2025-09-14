add_executable(delete_test delete_test.cpp)
add_executable(redirect_test redirect_test.cpp)
add_executable(static_test static_test.cpp)
add_executable(upload_test upload_test.cpp)
add_executable(cgi_test cgi_test.cpp)

# 依存関係を追加
add_dependencies(delete_test webserv_lib)
add_dependencies(redirect_test webserv_lib)
add_dependencies(static_test webserv_lib)
add_dependencies(upload_test webserv_lib)
add_dependencies(cgi_test webserv_lib)

# ライブラリをリンク
target_link_libraries(delete_test PRIVATE webserv_lib gtest_main gmock_main)
target_link_libraries(redirect_test PRIVATE webserv_lib gtest_main gmock_main)
target_link_libraries(static_test PRIVATE webserv_lib gtest_main gmock_main)
target_link_libraries(upload_test PRIVATE webserv_lib gtest_main gmock_main)
target_link_libraries(cgi_test PRIVATE webserv_lib gtest_main gmock_main)

# テストを検出
gtest_discover_tests(delete_test)
gtest_discover_tests(redirect_test)
gtest_discover_tests(static_test)
gtest_discover_tests(upload_test)
gtest_discover_tests(cgi_test)
