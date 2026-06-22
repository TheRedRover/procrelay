add_library(procrelay_log STATIC
    src/log.cpp
)

target_include_directories(procrelay_log
    PUBLIC include
)

target_compile_features(procrelay_log PUBLIC cxx_std_20)
