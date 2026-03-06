cmake_minimum_required(VERSION 3.11)
project(Aronkanoid VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
