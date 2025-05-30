cmake_minimum_required(VERSION 3.14)
project(WebServ)

set(CMAKE_CXX_STANDARD 17)

# GoogleTestの取得・利用
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# オプション（内部でビルドする）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 本体コードをライブラリとして定義
add_library(my_math STATIC
    src/math.cpp
    src/math.h
)

# テストコードのビルド
enable_testing()

add_executable(test_math
    unit_tests/test_math.cpp
)

# テスト実行ファイルに必要なライブラリをリンク
target_link_libraries(test_math
    gtest_main
    my_math
)

# GoogleTest によるテストの自動登録
include(GoogleTest)
gtest_discover_tests(test_math)
