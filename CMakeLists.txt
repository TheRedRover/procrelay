cmake_minimum_required(VERSION 3.20)
project(procrelay CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

set(PROCRELAY_VERSION "")
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE PROCRELAY_GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE PROCRELAY_GIT_RESULT
        ERROR_QUIET)
    if(PROCRELAY_GIT_RESULT EQUAL 0 AND PROCRELAY_GIT_VERSION)
        set(PROCRELAY_VERSION "${PROCRELAY_GIT_VERSION}")
    endif()
endif()
if(NOT PROCRELAY_VERSION AND EXISTS "${CMAKE_SOURCE_DIR}/VERSION")
    file(READ "${CMAKE_SOURCE_DIR}/VERSION" PROCRELAY_VERSION)
    string(STRIP "${PROCRELAY_VERSION}" PROCRELAY_VERSION)
endif()
if(NOT PROCRELAY_VERSION)
    set(PROCRELAY_VERSION "unknown")
endif()
message(STATUS "procrelay version: ${PROCRELAY_VERSION}")

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
