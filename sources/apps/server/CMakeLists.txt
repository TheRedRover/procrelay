add_executable(procrelay-server src/main.cpp)
target_link_libraries(procrelay-server
    PRIVATE
        procrelay_lib
        procrelay_log
        httplib::httplib
        nlohmann_json::nlohmann_json
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/procrelay/version.hpp
    @ONLY
)
target_include_directories(procrelay-server PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

if(PROCRELAY_STATIC)
    target_link_options(procrelay-server PRIVATE -static)
endif()
