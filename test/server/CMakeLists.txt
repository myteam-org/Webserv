# インクルードディレクトリの指定（より明示的に）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/server
    ${CMAKE_SOURCE_DIR}/include/server/socket
    ${CMAKE_SOURCE_DIR}/include/server/connection
	${CMAKE_SOURCE_DIR}/include/io/base
)

add_executable(epollEvent_test epollEvent_test.cpp)
target_link_libraries(epollEvent_test 
    PRIVATE webserv_lib gtest
)
gtest_discover_tests(epollEvent_test)

add_executable(epollEventNotifier_test epollEventNotifier_test.cpp)
target_link_libraries(epollEventNotifier_test PRIVATE webserv_lib gtest)
gtest_discover_tests(epollEventNotifier_test)

add_subdirectory(socket)
add_subdirectory(connection)
