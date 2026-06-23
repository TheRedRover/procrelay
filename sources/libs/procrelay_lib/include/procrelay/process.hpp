/**
 * @file process.hpp
 * @brief Defines the ProcessInfo class and related enums.
 */

#ifndef PROCRELAY_PROCESS_HPP
#define PROCRELAY_PROCESS_HPP

#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace procrelay
{

enum class ProcessState {
    RUNNING,
    SLEEPING,
    DISK_SLEEP,
    ZOMBIE,
    STOPPED,
    TRACING_STOP,
    DEAD,
    PARKED,
    IDLE,
    UNKNOWN,
};

ProcessState                state_from_char(char c);
std::string                 label_from_state(ProcessState s);
std::optional<ProcessState> state_from_label(std::string_view label);

/**
 * @brief Returns a string of all valid state labels.
 *
 * @return const std::string& A string of all valid state labels, separated by commas.
 */
const std::string &valid_state_labels();

class ProcessInfo
{
public:
    ProcessInfo() = default;
    ProcessInfo(int pid, int ppid, std::string comm, std::vector<std::string> cmdline,
                char state_code, double cpu_time_s, std::optional<int64_t> start_time,
                std::optional<std::string> start_time_iso);

    int                             get_pid() const { return m_pid; }
    int                             get_ppid() const { return m_ppid; }
    const std::string              &get_comm() const { return m_comm; }
    const std::vector<std::string> &get_cmdline() const { return m_cmdline; }
    char                            get_state_code() const { return m_state_code; }
    ProcessState                    get_state() const { return m_state; }
    std::string                     get_state_label() const { return label_from_state(m_state); }
    double                          get_cpu_time_s() const { return m_cpu_time_s; }
    std::optional<int64_t>          get_start_time() const { return m_start_time; }
    std::optional<std::string>      get_start_time_iso() const { return m_start_time_iso; }

    nlohmann::json to_json_obj() const;
    std::string    to_json() const;

private:
    int                        m_pid{-1};
    int                        m_ppid{-1};
    std::string                m_comm;
    std::vector<std::string>   m_cmdline;
    char                       m_state_code{0};
    ProcessState               m_state{ProcessState::UNKNOWN};
    double                     m_cpu_time_s{0.0};
    std::optional<int64_t>     m_start_time{};
    std::optional<std::string> m_start_time_iso{};
};

} // namespace procrelay

#endif // PROCRELAY_PROCESS_HPP
