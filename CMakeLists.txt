cmake_minimum_required(VERSION 3.14)
project(WebServ)

# compile_commands.jsonの生成
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# バイナリ出力先の基本設定
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# テストバイナリ用の追加設定
set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

# テスト実行ファイルの出力先を設定
set(CMAKE_TEST_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

# includeディレクトリの設定
include_directories(${CMAKE_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

find_program(VALGRIND valgrind)

add_executable(webserv
	src/config/Token.cpp
	src/config/Config.cpp
	src/config/ConfigParser.cpp
	src/config/ConfigTokenizer.cpp
	src/config/ServerContext.cpp
	src/validator/Validator.cpp
	src/main.cpp
)

add_custom_target(memcheck
	COMMAND ${VALGRIND} --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 $<TARGET_FILE:webserv>
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS webserv
)

# GoogleTestの取得・利用
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# オプション（内部でビルドする）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)



# 以下はサンプルコード。unit_test_sample　以下参照。実際の実装時には置き換える必要あり。
# 本体コードをライブラリとして定義。ソースは肥大化するためsources.cmakeから取得する。

include(unit_test_sample/srcs/sources.cmake)
add_library(my_math STATIC
	${MY_MATH_SOURCES}
	${MY_MATH_HEADERS}
)

# テストコードのソースファイルを指定
add_executable(test_math
    unit_test_sample/tests/test_math.cpp
)

set_target_properties(test_math PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_TEST_OUTPUT_DIRECTORY}
)

# テスト実行ファイルに必要なライブラリをリンク
target_link_libraries(test_math
    gtest_main
    my_math
)

# GoogleTest によるテストの自動登録
include(GoogleTest)
gtest_discover_tests(test_math)
