include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/utils/type
    ${CMAKE_SOURCE_DIR}/include/server/connection
    ${CMAKE_SOURCE_DIR}/include/server/connection/state
    ${CMAKE_SOURCE_DIR}/include/server/socket
    ${CMAKE_SOURCE_DIR}/include/io/input/read
    ${CMAKE_SOURCE_DIR}/include/io/input/write
    ${CMAKE_SOURCE_DIR}/include/http/request
    ${CMAKE_SOURCE_DIR}/include/http/response
)

add_executable(connection_test connection_test.cpp)
target_link_libraries(connection_test 
PRIVATE webserv_lib gtest
)
gtest_discover_tests(connection_test)

add_executable(connectionManager_test connectionManager_test.cpp)
target_link_libraries(connectionManager_test 
PRIVATE webserv_lib gtest
)
gtest_discover_tests(connectionManager_test)
