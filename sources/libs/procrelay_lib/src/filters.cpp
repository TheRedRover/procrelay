#include "procrelay/filters.hpp"

#include "procrelay/log.hpp"

#include <algorithm>
#include <charconv>
#include <utility>
#include <variant>

namespace procrelay
{

namespace
{

/**
 * @brief Parses a string as an int64_t.
 *
 * @param s The string to parse.
 * @return std::optional<int64_t> The parsed value, or std::nullopt if the string is not valid int
 */
std::optional<int64_t> parse_epoch(const std::string &s)
{
    int64_t     value = 0;
    const char *begin = s.data();
    const char *end   = s.data() + s.size();
    auto [ptr, ec]    = std::from_chars(begin, end, value);
    if (ec != std::errc{} || ptr != end) {
        return std::nullopt;
    }
    return value;
}

// Overload apply_one for each AnyFilter

std::vector<ProcessInfo> apply_one(std::vector<ProcessInfo> procs, const StateFilter &f)
{
    procs.erase(std::remove_if(procs.begin(), procs.end(),
                               [&](const ProcessInfo &p) { return p.get_state() != f.m_state; }),
                procs.end());
    return procs;
}

std::vector<ProcessInfo> apply_one(std::vector<ProcessInfo> procs, const TimeRangeFilter &f)
{
    procs.erase(std::remove_if(procs.begin(), procs.end(),
                               [&](const ProcessInfo &p) {
                                   const std::optional<int64_t> start = p.get_start_time();
                                   if (!start.has_value()) {
                                       // Exclude if start time is not available
                                       return true;
                                   }
                                   const int64_t t = *start;
                                   return (f.m_after.has_value() && t < *f.m_after) ||
                                          (f.m_before.has_value() && t > *f.m_before);
                               }),
                procs.end());
    return procs;
}

} // namespace

ParseResult parse_filters(const FilterParams &params)
{
    FilterChain chain;

    if (params.state.has_value()) {
        auto state = state_from_label(*params.state);
        if (!state.has_value()) {
            log::debug("rejecting filters: unknown state '", *params.state, "'");
            return FilterError{"invalid_state", "unknown state '" + *params.state +
                                                    "'; valid: " + valid_state_labels()};
        }
        chain.push_back(StateFilter{*state});
    }

    if (params.started_after.has_value() || params.started_before.has_value()) {
        TimeRangeFilter tf;

        if (params.started_after.has_value()) {
            auto value = parse_epoch(*params.started_after);
            if (!value.has_value()) {
                log::debug("rejecting filters: started_after='", *params.started_after, "'");
                return FilterError{"invalid_time", "started_after is not a valid integer"};
            }
            tf.m_after = *value;
        }

        if (params.started_before.has_value()) {
            auto value = parse_epoch(*params.started_before);
            if (!value.has_value()) {
                log::debug("rejecting filters: started_before='", *params.started_before, "'");
                return FilterError{"invalid_time", "started_before is not a valid integer"};
            }
            tf.m_before = *value;
        }

        if (tf.m_after.has_value() && tf.m_before.has_value() && *tf.m_after > *tf.m_before) {
            log::debug("rejecting filters: started_after > started_before");
            return FilterError{"invalid_range", "started_after must be <= started_before"};
        }

        chain.push_back(tf);
    }

    log::debug("parsed ", chain.size(), " filter(s)");
    return chain;
}

std::vector<ProcessInfo> apply_filters(const std::vector<ProcessInfo> &procs,
                                       const FilterChain              &chain)
{
    std::vector<ProcessInfo> result = procs;
    for (const auto &filter : chain) {
        const std::size_t before = result.size();
        result = std::visit([&](const auto &f) { return apply_one(std::move(result), f); }, filter);
        log::debug("filter narrowed ", before, " -> ", result.size(), " processes");
    }
    return result;
}

} // namespace procrelay
