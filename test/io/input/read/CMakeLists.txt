# インクルードディレクトリの指定（より明示的に）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/type
    ${CMAKE_SOURCE_DIR}/include/io/input/read
)

add_executable(buffer_test buffer_test.cpp)
target_link_libraries(buffer_test 
    PRIVATE webserv_lib gtest
)

# テストディスカバリー
gtest_discover_tests(buffer_test)

