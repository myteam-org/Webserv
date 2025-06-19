# インクルードディレクトリの指定（より明示的に）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils
)

# Result用のテスト実行ファイル
add_executable(result_test result_test.cpp)
target_link_libraries(result_test 
    PRIVATE webserv_lib gtest
)

# Option用のテスト実行ファイル
add_executable(option_test option_test.cpp)
target_link_libraries(option_test
	PRIVATE webserv_lib gtest
)

# テストディスカバリー
gtest_discover_tests(result_test)
gtest_discover_tests(option_test)
