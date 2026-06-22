
/**
 * @file collector.hpp
 * @brief Declares functions for collecting process information from the /proc filesystem.
 */

#ifndef PROCRELAY_COLLECTOR_HPP
#define PROCRELAY_COLLECTOR_HPP

#include "process.hpp"

#include <filesystem>
#include <optional>
#include <vector>

namespace procrelay
{

std::vector<ProcessInfo>   scan(const std::filesystem::path &proc_root = "/proc");
std::optional<ProcessInfo> get_process(int                          pid,
                                       const std::filesystem::path &proc_root = "/proc");

} // namespace procrelay

#endif // PROCRELAY_COLLECTOR_HPP
