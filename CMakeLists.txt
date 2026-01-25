cmake_minimum_required(VERSION 3.10)

project(mswpr)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE you_werent_supposed_to_be_here)

include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(raylib)

file(GLOB SOURCES "src/*.c")

add_executable(mswpr ${SOURCES})
include_directories("vendors/ystar/dist")

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
target_compile_options(mswpr PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(mswpr PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(mswpr PRIVATE raylib)
