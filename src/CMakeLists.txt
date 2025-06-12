# set c++98
set(CMAKE_CXX_STANDARD 98)
# c++98完全準拠
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# コンパイラ固有の拡張機能無効
set(CMAKE_CXX_EXTENSIONS OFF)

# 実行ファイルをプロジェクトルートに出力
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# webserv実行ファイルの定義
add_library(webserv_lib STATIC
    config/Token.cpp
    config/Config.cpp
    config/ConfigParser.cpp
    config/ConfigTokenizer.cpp
    config/ServerContext.cpp
	config/LocationContext.cpp
    validator/Validator.cpp
)

# includeディレクトリの設定（ターゲットベース）
target_include_directories(webserv_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 実行ファイルの生成
add_executable(webserv main.cpp)

# 実行ファイルとライブラリの結合
target_link_libraries(webserv PRIVATE webserv_lib)
