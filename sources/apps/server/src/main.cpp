#include "procrelay/collector.hpp"
#include "procrelay/filters.hpp"
#include "procrelay/log.hpp"
#include "procrelay/version.hpp"

#include <charconv>
#include <cstdlib>
#include <getopt.h>
#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace
{

using namespace procrelay;

struct Config {
    std::string bind      = "0.0.0.0";
    int         port      = 8080;
    std::string proc_root = "/proc";
    std::string log_level = "info";
};

std::optional<std::string> get_env(const char *name)
{
    if (const char *value = std::getenv(name)) {
        return std::string(value);
    }
    return std::nullopt;
}

std::optional<int> parse_port(const std::string &s)
{
    int value      = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size() || value < 1 || value > 65535) {
        return std::nullopt;
    }
    return value;
}

void print_version()
{
    std::cout << "procrelay " << procrelay::VERSION << "\n";
}

void print_usage(const char *prog)
{
    std::cout
        << "procrelay " << procrelay::VERSION << "\n\n"
        << "Usage: " << prog << " [options]\n\n"
        << "Options:\n"
        << "  --port <n>         TCP port to listen on (default 8080, env PROCRELAY_PORT)\n"
        << "  --bind <addr>      Address to bind to (default 0.0.0.0, env PROCRELAY_BIND)\n"
        << "  --proc-root <dir>  Root path for /proc reads (default /proc, env "
           "PROCRELAY_PROC_ROOT)\n"
        << "  --log-level <lvl>  debug|info|warn|error (default info, env PROCRELAY_LOG_LEVEL)\n"
        << "  -v, --version      Show version and exit\n"
        << "  -h, --help         Show this help and exit\n";
}

bool load_config(int argc, char **argv, Config &cfg)
{
    if (auto v = get_env("PROCRELAY_BIND"))
        cfg.bind = *v;
    if (auto v = get_env("PROCRELAY_PROC_ROOT"))
        cfg.proc_root = *v;
    if (auto v = get_env("PROCRELAY_LOG_LEVEL"))
        cfg.log_level = *v;
    std::optional<std::string> port_raw = get_env("PROCRELAY_PORT");

    enum { OPT_PORT = 256, OPT_BIND, OPT_PROC_ROOT, OPT_LOG_LEVEL };
    static const option longopts[] = {
        {"port", required_argument, nullptr, OPT_PORT},
        {"bind", required_argument, nullptr, OPT_BIND},
        {"proc-root", required_argument, nullptr, OPT_PROC_ROOT},
        {"log-level", required_argument, nullptr, OPT_LOG_LEVEL},
        {"version", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0},
    };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "hv", longopts, nullptr)) != -1) {
        switch (opt) {
        case OPT_PORT:
            port_raw = optarg;
            break;
        case OPT_BIND:
            cfg.bind = optarg;
            break;
        case OPT_PROC_ROOT:
            cfg.proc_root = optarg;
            break;
        case OPT_LOG_LEVEL:
            cfg.log_level = optarg;
            break;
        case 'v':
            print_version();
            std::exit(0);
        case 'h':
            print_usage(argv[0]);
            std::exit(0);
        default:
            return false;
        }
    }

    if (optind < argc) {
        log::error("unexpected argument: ", argv[optind]);
        return false;
    }

    if (port_raw.has_value()) {
        auto parsed = parse_port(*port_raw);
        if (!parsed.has_value()) {
            log::error("invalid port '", *port_raw, "'; expected 1-65535");
            return false;
        }
        cfg.port = *parsed;
    }

    if (!log::level_from_string(cfg.log_level).has_value()) {
        log::error("invalid log level '", cfg.log_level, "'; expected debug|info|warn|error");
        return false;
    }

    return true;
}

std::string error_body(const std::string &code, const std::string &message)
{
    nlohmann::json j;
    j["error"] = {{"code", code}, {"message", message}};
    return j.dump();
}

void send_error(httplib::Response &res, int status, const std::string &code,
                const std::string &message)
{
    res.status = status;
    res.set_content(error_body(code, message), "application/json");
}

std::string process_list_body(const std::vector<ProcessInfo> &procs)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto &proc : procs) {
        arr.push_back(proc.to_json_obj());
    }
    nlohmann::json j;
    j["processes"] = std::move(arr);
    j["count"]     = procs.size();
    return j.dump();
}

std::optional<int> parse_positive_int(const std::string &s)
{
    int value      = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size() || value < 1) {
        return std::nullopt;
    }
    return value;
}

} // namespace

int main(int argc, char **argv)
{
    Config cfg;
    if (!load_config(argc, argv, cfg)) {
        return 1;
    }
    log::set_level(*log::level_from_string(cfg.log_level));

    httplib::Server svr;

    svr.Get("/v1/processes", [&cfg](const httplib::Request &req, httplib::Response &res) {
        FilterParams params;
        if (req.has_param("state"))
            params.state = req.get_param_value("state");
        if (req.has_param("started_after"))
            params.started_after = req.get_param_value("started_after");
        if (req.has_param("started_before"))
            params.started_before = req.get_param_value("started_before");

        ParseResult parsed = parse_filters(params);
        if (const auto *err = std::get_if<FilterError>(&parsed)) {
            send_error(res, 400, err->m_code, err->m_message);
            return;
        }

        const auto &chain = std::get<FilterChain>(parsed);
        auto        procs = apply_filters(scan(cfg.proc_root), chain);
        res.set_content(process_list_body(procs), "application/json");
    });

    svr.Get(
        R"(/v1/processes/([^/]+))", [&cfg](const httplib::Request &req, httplib::Response &res) {
            const std::string pid_str = req.matches[1];
            auto              pid     = parse_positive_int(pid_str);
            if (!pid.has_value()) {
                send_error(res, 400, "invalid_pid", "pid must be a positive integer");
                return;
            }

            auto proc = get_process(*pid, cfg.proc_root);
            if (!proc.has_value()) {
                send_error(res, 404, "not_found", "no process with pid " + std::to_string(*pid));
                return;
            }
            res.set_content(proc->to_json(), "application/json");
        });

    // According to OpenAPI, non-GET ones are not allowed
    auto method_not_allowed = [](const httplib::Request &req, httplib::Response &res) {
        res.set_header("Allow", "GET");
        send_error(res, 405, "method_not_allowed",
                   "method " + req.method + " is not allowed; use GET");
    };
    for (const char *path : {"/v1/processes", R"(/v1/processes/([^/]+))"}) {
        svr.Post(path, method_not_allowed);
        svr.Put(path, method_not_allowed);
        svr.Delete(path, method_not_allowed);
        svr.Patch(path, method_not_allowed);
        svr.Options(path, method_not_allowed);
    }

    // Anything unmatched is an unknown route.
    svr.set_error_handler([](const httplib::Request &, httplib::Response &res) {
        if (res.body.empty()) {
            res.status = 404;
            res.set_content(error_body("not_found_route", "route not found"), "application/json");
        }
    });

    svr.set_logger([](const httplib::Request &req, const httplib::Response &res) {
        log::info(req.method, " ", req.path, " -> ", res.status);
    });

    log::info("procrelay ", procrelay::VERSION, " listening on ", cfg.bind, ":", cfg.port,
              " (proc-root=", cfg.proc_root, ")");

    if (!svr.listen(cfg.bind, cfg.port)) {
        log::error("failed to bind ", cfg.bind, ":", cfg.port);
        return 1;
    }
    return 0;
}
