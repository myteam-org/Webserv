cmake_minimum_required(VERSION 3.14)
project(webserv)

include(CTest)

# compile_commands.jsonの生成
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# サブディレクトリを追加
add_subdirectory(src)
add_subdirectory(test)


# clang-tidyの設定
find_program(CLANG_TIDY clang-tidy)
find_program(GIT git)
set(CLANG_TIDY_SCRIPT "${CMAKE_BINARY_DIR}/run_clang_tidy.sh")
file(WRITE ${CLANG_TIDY_SCRIPT} "#!/bin/bash\n")
file(APPEND ${CLANG_TIDY_SCRIPT} "cd \"${CMAKE_SOURCE_DIR}\"\n")
file(APPEND ${CLANG_TIDY_SCRIPT} "${GIT} ls-files '*.cpp' '*.hpp' | grep -vE '(^test/|^build/)' | xargs ${CLANG_TIDY} -p build\n")

add_custom_target(clang-tidy
	COMMAND chmod +x ${CLANG_TIDY_SCRIPT}
	COMMAND ${CLANG_TIDY_SCRIPT}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# valgrindの設定
find_program(VALGRIND valgrind)
add_custom_target(memcheck
    COMMAND ${VALGRIND} --leak-check=full --show-leak-kinds=all --track-origins=yes --error-exitcode=1 $<TARGET_FILE:webserv>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS webserv
)
