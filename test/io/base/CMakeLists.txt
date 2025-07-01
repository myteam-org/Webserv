# インクルードディレクトリの指定（より明示的に）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/type
    ${CMAKE_SOURCE_DIR}/include/io/base
)

add_executable(fileDescriptor_test fileDescriptor_test.cpp)
target_link_libraries(fileDescriptor_test 
    PRIVATE webserv_lib gtest
)

# テストディスカバリー
gtest_discover_tests(fileDescriptor_test)

