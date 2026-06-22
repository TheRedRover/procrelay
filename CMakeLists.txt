cmake_minimum_required(VERSION 3.20)
project(procrelay CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

enable_testing()

add_subdirectory(sources/libs/procrelay_lib)
add_subdirectory(sources/apps/server)
add_subdirectory(tests)
