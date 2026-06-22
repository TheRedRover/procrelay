#include <procrelay/process.hpp>

#include <array>
#include <cctype>
#include <nlohmann/json.hpp>

namespace procrelay
{

namespace
{

struct StateMapping {
    ProcessState m_state;
    const char  *m_label;
};

// Canonical state <-> label table. label_from_state and state_from_label share it.
constexpr std::array<StateMapping, 10> STATE_TABLE{{
    {ProcessState::RUNNING, "running"},
    {ProcessState::SLEEPING, "sleeping"},
    {ProcessState::DISK_SLEEP, "disk-sleep"},
    {ProcessState::ZOMBIE, "zombie"},
    {ProcessState::STOPPED, "stopped"},
    {ProcessState::TRACING_STOP, "tracing-stop"},
    {ProcessState::DEAD, "dead"},
    {ProcessState::PARKED, "parked"},
    {ProcessState::IDLE, "idle"},
    {ProcessState::UNKNOWN, "unknown"},
}};

} // namespace

ProcessInfo::ProcessInfo(int pid, int ppid, std::string comm, std::vector<std::string> cmdline,
                         char state_code, double cpu_time_s, int64_t start_time,
                         std::string start_time_iso)
    : m_pid(pid), m_ppid(ppid), m_comm(std::move(comm)), m_cmdline(std::move(cmdline)),
      m_state_code(state_code), m_state(state_from_char(state_code)), m_cpu_time_s(cpu_time_s),
      m_start_time(start_time), m_start_time_iso(std::move(start_time_iso))
{
}

ProcessState state_from_char(char c)
{
    switch (c) {
    case 'R': return ProcessState::RUNNING;
    case 'S': return ProcessState::SLEEPING;
    case 'D': return ProcessState::DISK_SLEEP;
    case 'Z': return ProcessState::ZOMBIE;
    case 'T': return ProcessState::STOPPED;
    case 't': return ProcessState::TRACING_STOP;
    case 'X':
    case 'x': return ProcessState::DEAD;
    case 'I': return ProcessState::IDLE;
    case 'P': return ProcessState::PARKED;
    default: return ProcessState::UNKNOWN;
    }
}

std::string label_from_state(ProcessState state)
{
    for (const auto &entry : STATE_TABLE) {
        if (entry.m_state == state) {
            return entry.m_label;
        }
    }
    return "unknown";
}

std::optional<ProcessState> state_from_label(std::string_view label)
{
    std::string lowered;
    lowered.reserve(label.size());
    for (char c : label) {
        lowered += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    for (const auto &entry : STATE_TABLE) {
        // UNKNOWN is an internal sentinel, not a valid input label.
        if (entry.m_state == ProcessState::UNKNOWN) {
            continue;
        }
        if (lowered == entry.m_label) {
            return entry.m_state;
        }
    }
    return std::nullopt;
}

std::string ProcessInfo::to_json() const
{
    nlohmann::json j;
    j["pid"]            = m_pid;
    j["ppid"]           = m_ppid;
    j["comm"]           = m_comm;
    j["cmdline"]        = m_cmdline;
    j["state"]          = std::string(1, m_state_code);
    j["state_label"]    = label_from_state(m_state);
    j["cpu_time_s"]     = m_cpu_time_s;
    j["start_time"]     = m_start_time;
    j["start_time_iso"] = m_start_time_iso;
    return j.dump();
}

} // namespace procrelay
