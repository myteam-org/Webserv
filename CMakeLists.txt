cmake_minimum_required(VERSION 3.14)
project(webserv)

include(CTest)

# compile_commands.jsonの生成
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# サブディレクトリを追加
add_subdirectory(src)
add_subdirectory(test)

# valgrindの設定
# cmake --build build --target memcheck
find_program(VALGRIND valgrind)
add_custom_target(memcheck
	COMMAND ${VALGRIND} --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 $<TARGET_FILE:webserv>
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS webserv
)
