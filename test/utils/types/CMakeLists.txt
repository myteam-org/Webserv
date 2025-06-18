# インクルードディレクトリの指定（より明示的に）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils
)

# 実行ファイルの作成
add_executable(result_test result_test.cpp)

# 依存関係とリンク
target_link_libraries(result_test 
    PRIVATE 
    webserv_lib 
    gtest 
    gtest_main 
    gmock 
    gmock_main
)

# テストディスカバリー
gtest_discover_tests(result_test)
