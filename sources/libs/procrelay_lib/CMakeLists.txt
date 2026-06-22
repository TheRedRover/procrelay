add_library(procrelay_lib STATIC
    src/process.cpp
    src/collector.cpp
    src/filters.cpp
)

target_include_directories(procrelay_lib
    PUBLIC include
)

target_link_libraries(procrelay_lib
    PRIVATE
        nlohmann_json::nlohmann_json
)

target_compile_features(procrelay_lib PUBLIC cxx_std_20)
