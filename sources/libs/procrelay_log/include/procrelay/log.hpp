#ifndef PROCRELAY_LOG_HPP
#define PROCRELAY_LOG_HPP

#include <optional>
#include <sstream>
#include <string_view>
#include <utility>

namespace procrelay::log
{

enum class Level { DEBUG, INFO, WARN, ERROR };

void  set_level(Level level);
Level get_level();

std::optional<Level> level_from_string(std::string_view name);
const char          *level_name(Level level);

void write(Level level, std::string_view message);

template <typename... Args> void message(Level level, Args &&...args)
{
    if (static_cast<int>(level) < static_cast<int>(get_level())) {
        return;
    }
    std::ostringstream os;
    (os << ... << std::forward<Args>(args));
    write(level, os.str());
}

template <typename... Args> void debug(Args &&...args)
{
    message(Level::DEBUG, std::forward<Args>(args)...);
}

template <typename... Args> void info(Args &&...args)
{
    message(Level::INFO, std::forward<Args>(args)...);
}

template <typename... Args> void warn(Args &&...args)
{
    message(Level::WARN, std::forward<Args>(args)...);
}

template <typename... Args> void error(Args &&...args)
{
    message(Level::ERROR, std::forward<Args>(args)...);
}

} // namespace procrelay::log

#endif // PROCRELAY_LOG_HPP
