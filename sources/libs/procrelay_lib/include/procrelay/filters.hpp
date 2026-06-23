/**
 * @file filters.hpp
 * @brief Defines the filtering logic for process information.
 */

#ifndef PROCRELAY_FILTERS_HPP
#define PROCRELAY_FILTERS_HPP

#include "process.hpp"

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace procrelay
{

struct FilterError {
    std::string m_code;
    std::string m_message;
};

struct StateFilter {
    ProcessState m_state;
};

// started_after and started_before are kept together.
// Cross-validation requires seeing both values at once.
struct TimeRangeFilter {
    std::optional<int64_t> m_after;
    std::optional<int64_t> m_before;
};

using AnyFilter   = std::variant<StateFilter, TimeRangeFilter>;
using FilterChain = std::vector<AnyFilter>;
using ParseResult = std::variant<FilterChain, FilterError>;

// Adding a new filter param means adding a field here
struct FilterParams {
    std::optional<std::string> state          = std::nullopt;
    std::optional<std::string> started_after  = std::nullopt;
    std::optional<std::string> started_before = std::nullopt;
};

/**
 * @brief Parse the filter parameters
 *
 * @param params The filter parameters
 * @return ParseResult A ParseResult object, or a FilterError if the filter parameters are invalid
 */
ParseResult parse_filters(const FilterParams &params);

/**
 * @brief Apply the filters to a list of ProcessInfo objects
 *
 * @param procs The list of ProcessInfo objects
 * @param chain The filter chain
 * @return std::vector<ProcessInfo> A list of ProcessInfo objects that match the filters
 */
std::vector<ProcessInfo> apply_filters(const std::vector<ProcessInfo> &procs,
                                       const FilterChain              &chain);

} // namespace procrelay

#endif // PROCRELAY_FILTERS_HPP
