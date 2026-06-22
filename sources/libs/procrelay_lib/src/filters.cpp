#include <procrelay/filters.hpp>

namespace procrelay
{

ParseResult parse_filters(const FilterParams & /*params*/)
{
    // TODO: implement
    return FilterChain{};
}

std::vector<ProcessInfo> apply_filters(const std::vector<ProcessInfo> & /*procs*/,
                                       const FilterChain & /*chain*/)
{
    // TODO: implement
    return {};
}

} // namespace procrelay
