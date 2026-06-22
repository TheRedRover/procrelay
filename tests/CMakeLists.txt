include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(procrelay-tests
    unit/test_state.cpp
    unit/test_filters.cpp
    unit/test_collector.cpp
)

target_link_libraries(procrelay-tests
    PRIVATE
        procrelay_lib
        GTest::gtest_main
)

target_compile_definitions(procrelay-tests PRIVATE
    FIXTURE_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures"
)

include(GoogleTest)
gtest_discover_tests(procrelay-tests)
