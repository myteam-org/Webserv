include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/type
    ${CMAKE_SOURCE_DIR}/include/server/socket
    ${CMAKE_SOURCE_DIR}/include/io/base
)

add_executable(connectionSocket_test connectionSocket_test.cpp)
target_link_libraries(connectionSocket_test 
PRIVATE webserv_lib gtest
)
gtest_discover_tests(connectionSocket_test)

add_executable(serverSocket_test serverSocket_test.cpp)
target_link_libraries(serverSocket_test 
PRIVATE webserv_lib gtest
)
gtest_discover_tests(serverSocket_test)

add_executable(socketAddr_test socketAddr_test.cpp)
target_link_libraries(socketAddr_test 
PRIVATE webserv_lib gtest
)
gtest_discover_tests(socketAddr_test)

