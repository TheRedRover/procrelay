cmake_minimum_required(VERSION 3.20)
project(procrelay CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

option(PROCRELAY_BUILD_TESTS "Build the unit test suite" ON)
option(PROCRELAY_STATIC "Link executables statically (for scratch-based images)" OFF)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)

add_subdirectory(sources/libs/procrelay_log)
add_subdirectory(sources/libs/procrelay_lib)
add_subdirectory(sources/apps/server)

if(PROCRELAY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
