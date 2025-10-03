# testなのでc++20使っても良い
# set(CMAKE_CXX_STANDARD 20)

# GoogleTestの取得・利用
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # v1.14.0
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

set(BUILD_GMOCK ON CACHE BOOL "" FORCE)


add_subdirectory(config)
add_subdirectory(utils)
add_subdirectory(io)
add_subdirectory(server)
add_subdirectory(http)
