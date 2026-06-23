
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

/**
 * @brief Scan the /proc filesystem for all processes and return a vector of ProcessInfo objects
 *
 * @param proc_root The path to the proc root
 * @return std::vector<ProcessInfo> A vector of ProcessInfo objects
 */
std::vector<ProcessInfo> scan(const std::filesystem::path &proc_root = "/proc");

/**
 * @brief Get a ProcessInfo object for a given process ID
 *
 * @param pid The process ID
 * @param proc_root The path to the proc root
 * @return std::optional<ProcessInfo> A ProcessInfo object, or std::nullopt if the process does not
 * exist
 */
std::optional<ProcessInfo> get_process(int pid, const std::filesystem::path &proc_root = "/proc");

} // namespace procrelay

#endif // PROCRELAY_COLLECTOR_HPP
