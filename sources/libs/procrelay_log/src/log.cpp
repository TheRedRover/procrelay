#include "procrelay/log.hpp"

#include <atomic>
#include <iostream>

namespace procrelay::log
{

namespace
{

std::atomic<int> g_level{static_cast<int>(Level::INFO)};

} // namespace

void set_level(Level level)
{
    g_level.store(static_cast<int>(level), std::memory_order_relaxed);
}

Level get_level()
{
    return static_cast<Level>(g_level.load(std::memory_order_relaxed));
}

std::optional<Level> level_from_string(std::string_view name)
{
    if (name == "debug") {
        return Level::DEBUG;
    }
    if (name == "info") {
        return Level::INFO;
    }
    if (name == "warn") {
        return Level::WARN;
    }
    if (name == "error") {
        return Level::ERROR;
    }
    return std::nullopt;
}

const char *level_name(Level level)
{
    switch (level) {
    case Level::DEBUG:
        return "debug";
    case Level::INFO:
        return "info";
    case Level::WARN:
        return "warn";
    case Level::ERROR:
        return "error";
    }
    return "info";
}

void write(Level level, std::string_view message)
{
    if (static_cast<int>(level) < g_level.load(std::memory_order_relaxed)) {
        return;
    }
    std::cerr << "[" << level_name(level) << "] " << message << "\n";
}

} // namespace procrelay::log
