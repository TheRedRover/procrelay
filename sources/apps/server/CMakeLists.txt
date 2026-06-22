add_executable(procrelay-server src/main.cpp)
target_link_libraries(procrelay-server
    PRIVATE
        procrelay_lib
        procrelay_log
        httplib::httplib
        nlohmann_json::nlohmann_json
)
