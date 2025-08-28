enable_testing()

# gtest_discover_tests を使うなら必要
include(GoogleTest)

# まず find_package を試す（ある環境ではインポートターゲットになる）
find_package(GTest QUIET)

# 使うターゲット名を自動判定
if (TARGET GTest::gtest AND TARGET GTest::gtest_main)
  set(GTEST_LIB  GTest::gtest)
  set(GTEST_MAIN GTest::gtest_main)
elseif (TARGET gtest AND TARGET gtest_main)
  set(GTEST_LIB  gtest)
  set(GTEST_MAIN gtest_main)
else()
  message(FATAL_ERROR "GoogleTest targets not found.
  Install GoogleTest (find_package(GTest)) or add_subdirectory(googletest) first.")
endif()

# pthread が要る環境向け
find_package(Threads REQUIRED)
set(THREADS_LIB Threads::Threads)

# インクルードディレクトリの指定（より明示的に）
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/server
    ${CMAKE_SOURCE_DIR}/include/server/socket
    ${CMAKE_SOURCE_DIR}/include/server/connection
    ${CMAKE_SOURCE_DIR}/include/server/platform
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

add_executable(signal_layer_test platform/signal_layer_test.cpp)  # ← タイポ修正
target_link_libraries(signal_layer_test PRIVATE webserv_lib gtest)  # ← プロジェクトのgtestターゲット名に合わせて
gtest_discover_tests(signal_layer_test)

add_subdirectory(socket)
add_subdirectory(connection)
