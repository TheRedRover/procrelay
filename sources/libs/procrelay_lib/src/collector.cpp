#include <algorithm>
#include <cctype>
#include <ctime>
#include <fstream>
#include <procrelay/collector.hpp>
#include <sstream>
#include <string>
#include <unistd.h>

namespace procrelay
{

namespace
{

namespace fs = std::filesystem;

/**
 * @brief Reads a file into a string
 *
 * @param path The path to the file to read
 * @return std::optional<std::string>
 */
std::optional<std::string> read_file(const fs::path &path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }

    std::string content;
    char        buffer[4096];

    while (in.read(buffer, sizeof(buffer))) {
        content.append(buffer, static_cast<std::size_t>(in.gcount()));
    }

    content.append(buffer, static_cast<std::size_t>(in.gcount()));
    if (in.bad()) {
        return std::nullopt;
    }
    return content;
}

/**
 * @brief Checks if a string is a valid PID directory name
 *
 * @param name The directory name to check
 * @return true if the name is a valid PID directory name, false otherwise
 */
bool is_pid_dir_name(const std::string &name)
{
    return !name.empty() && std::all_of(name.begin(), name.end(),
                                        [](unsigned char c) { return std::isdigit(c) != 0; });
}

/**
 * @brief Extract the boot value time from /proc/stat
 *
 * @param proc_root The path to the proc root
 * @return std::optional<int64_t> The boot time, or std::nullopt if it cannot be read
 */
std::optional<int64_t> read_btime(const fs::path &proc_root)
{
    std::ifstream in(proc_root / "stat");
    if (!in) {
        return std::nullopt;
    }

    std::string key;
    while (in >> key) {
        if (key == "btime") {
            int64_t value = 0;
            if (in >> value) {
                return value;
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}

/**
 * @brief System-wide values that are constant for the duration of a single scan
 */
struct SystemContext {
    int64_t m_btime;
    long    m_clk_tck;
};

SystemContext make_system_context(const fs::path &proc_root)
{
    return SystemContext{read_btime(proc_root).value_or(0), ::sysconf(_SC_CLK_TCK)};
}

/**
 * @brief Splits the command line of a process into vector of strings
 *
 * @param pid_dir The path to the process directory
 * @return std::vector<std::string> The command line arguments
 */
std::vector<std::string> parse_cmdline(const fs::path &pid_dir)
{
    std::vector<std::string>   parts;
    std::optional<std::string> content = read_file(pid_dir / "cmdline");
    if (!content || content->empty()) {
        return parts;
    }

    std::string current;
    for (char c : *content) {
        if (c == '\0') {
            parts.push_back(std::move(current));
            current.clear();
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        parts.push_back(std::move(current));
    }
    return parts;
}

std::string make_iso8601_utc(int64_t epoch_seconds)
{
    std::time_t t = static_cast<std::time_t>(epoch_seconds);
    std::tm     tm{};
    if (gmtime_r(&t, &tm) == nullptr) {
        return {};
    }
    char buf[32];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm) == 0) {
        return {};
    }
    return buf;
}

/**
 * @brief Parses a single /proc/[pid] directory into a ProcessInfo.

 *
 * @param pid_dir Path to the process directory
 * @param ctx Cached system values
 * @return A populated ProcessInfo, or std::nullopt if a required file is missing or malformed
 */
std::optional<ProcessInfo> parse_process(const fs::path &pid_dir, const SystemContext &ctx)
{
    const auto stat = read_file(pid_dir / "stat");
    if (!stat) {
        return std::nullopt;
    }
    const std::string &content = *stat;

    auto lparen = content.find('(');
    auto rparen = content.rfind(')');

    if (lparen == std::string::npos || rparen == std::string::npos || rparen < lparen) {
        return std::nullopt;
    }

    int pid = 0;
    {
        std::istringstream pid_stream(content.substr(0, lparen));
        if (!(pid_stream >> pid)) {
            return std::nullopt;
        }
    }

    std::string comm = content.substr(lparen + 1, rparen - lparen - 1);

    std::istringstream       rest(content.substr(rparen + 1));
    std::vector<std::string> fields;
    std::string              token;
    while (rest >> token) {
        fields.push_back(token);
    }

    // Fields after comm are positional
    // state[0]=#3, ppid[1]=#4, utime[11]=#14, stime[12]=#15, starttime[19]=#22.
    if (fields.size() < 20) {
        return std::nullopt;
    }

    char    state_code = 0;
    int     ppid       = 0;
    int64_t utime      = 0;
    int64_t stime      = 0;
    int64_t starttime  = 0;
    try {
        state_code = fields[0][0];
        ppid       = std::stoi(fields[1]);
        utime      = std::stoll(fields[11]);
        stime      = std::stoll(fields[12]);
        starttime  = std::stoll(fields[19]);
    } catch (const std::exception &) {
        return std::nullopt;
    }

    double  cpu_time_s = (ctx.m_clk_tck > 0) ? static_cast<double>(utime + stime) /
                                                  static_cast<double>(ctx.m_clk_tck)
                                             : 0.0;
    int64_t start_time = ctx.m_clk_tck > 0 ? ctx.m_btime + starttime / ctx.m_clk_tck : ctx.m_btime;
    std::string              start_time_iso = make_iso8601_utc(start_time);
    std::vector<std::string> cmdline        = parse_cmdline(pid_dir);

    return ProcessInfo(pid, ppid, std::move(comm), std::move(cmdline), state_code, cpu_time_s,
                       start_time, std::move(start_time_iso));
}

} // namespace

std::vector<ProcessInfo> scan(const std::filesystem::path &proc_root)
{
    std::vector<ProcessInfo> result;

    std::error_code ec;
    if (!fs::is_directory(proc_root, ec)) {
        return result;
    }

    SystemContext ctx = make_system_context(proc_root);

    for (const auto &entry : fs::directory_iterator(proc_root, ec)) {
        if (ec) {
            break;
        }
        if (!entry.is_directory()) {
            continue;
        }
        if (!is_pid_dir_name(entry.path().filename().string())) {
            continue;
        }

        auto info = parse_process(entry.path(), ctx);
        if (info.has_value()) {
            result.push_back(std::move(*info));
        }
    }

    std::sort(result.begin(), result.end(),
              [](const ProcessInfo &a, const ProcessInfo &b) { return a.get_pid() < b.get_pid(); });

    return result;
}

std::optional<ProcessInfo> get_process(int pid, const std::filesystem::path &proc_root)
{
    std::error_code ec;
    fs::path        pid_dir = proc_root / std::to_string(pid);
    if (!fs::is_directory(pid_dir, ec)) {
        return std::nullopt;
    }

    return parse_process(pid_dir, make_system_context(proc_root));
}

} // namespace procrelay
