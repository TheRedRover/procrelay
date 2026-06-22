#include <procrelay/process.hpp>

namespace procrelay
{

ProcessInfo::ProcessInfo(int pid, int ppid, std::string comm, std::string cmdline, char state_code,
                         double cpu_time_s, int64_t start_time, std::string start_time_iso)
    : m_pid(pid), m_ppid(ppid), m_comm(std::move(comm)), m_cmdline(std::move(cmdline)),
      m_state_code(state_code), m_state(state_from_char(state_code)), m_cpu_time_s(cpu_time_s),
      m_start_time(start_time), m_start_time_iso(std::move(start_time_iso))
{
}

ProcessState state_from_char(char /*c*/)
{
    // TODO: implement
    return ProcessState::UNKNOWN;
}

std::string label_from_state(ProcessState /*s*/)
{
    // TODO: implement
    return "unknown";
}

std::optional<ProcessState> state_from_label(std::string_view /*label*/)
{
    // TODO: implement
    return std::nullopt;
}

std::string ProcessInfo::to_json() const
{
    // TODO: implement
    return "{}";
}

} // namespace procrelay
