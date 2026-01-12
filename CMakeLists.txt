cmake_minimum_required(VERSION 3.10)

project(mswpr)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE you_werent_supposed_to_be_here)

add_subdirectory("vendors/raylib")

file(GLOB SOURCES "src/*.c")

add_executable(mswpr ${SOURCES})
include_directories("vendors/ystar/dist")

target_compile_options(mswpr PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(mswpr PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(mswpr PRIVATE raylib)
