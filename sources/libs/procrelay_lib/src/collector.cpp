#include <procrelay/collector.hpp>

namespace procrelay
{

std::vector<ProcessInfo> scan(const std::filesystem::path & /*proc_root*/)
{
    // TODO: implement
    return {};
}

std::optional<ProcessInfo> get_process(int /*pid*/, const std::filesystem::path & /*proc_root*/)
{
    // TODO: implement
    return std::nullopt;
}

} // namespace procrelay