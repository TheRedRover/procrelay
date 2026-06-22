# procrelay

A lightweight per-node Linux process health agent. Reads `/proc` and exposes
process status over a REST API. Considered to be run on every node in a cluster.

## Installation

### Build natively

**Prerequisites:** GCC or Clang with C++20 support, CMake ≥ 3.20, Make or Ninja.

Build the image:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run the test suite:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Docker

The [`Dockerfile`](docker/Dockerfile) is multi-stage:

1. **build** -- compiles a fully static binary (`-DPROCRELAY_STATIC=ON`, tests skipped).
2. **deploy** -- copies that single binary onto `scratch`. The final image has no
   shell, libc, or package manager, runs as a non-root UID.

Build the image:

```bash
docker build -t procrelay -f docker/Dockerfile .
```

Run the image:

```bash
docker run --rm --pid=host -p 8080:8080 procrelay
```

Or with Docker Compose:

```bash
# To start
docker compose up -d
# To stop
docker compose down
```

> **Visibility note.** On a default host (`hidepid=0`) the non-root user can read
> every process's status files. On a host hardened with `hidepid=2`, a non-root
> container can't see other users' processes.
> Run as root (`docker run --user 0 ...` or `user: "0"` in compose)
> or add the container to the `proc` gid in that case.

## Usage

### Configuration

CLI flags take priority over environment variables. Both are optional.

| Flag          | Environment variable  | Default     | Description                         |
| ------------- | --------------------- | ----------- | ----------------------------------- |
| `--port`      | `PROCRELAY_PORT`      | `8080`      | TCP port to listen on               |
| `--bind`      | `PROCRELAY_BIND`      | `0.0.0.0`   | Address to bind to                  |
| `--proc-root` | `PROCRELAY_PROC_ROOT` | `/proc`     | Root path for `/proc` reads         |
| `--log-level` | `PROCRELAY_LOG_LEVEL` | `info`      | `debug` / `info` / `warn` / `error` |

```bash
# native
./procrelay --port 9090 --log-level debug
```

```bash
# Docker
docker run --rm --pid=host -p 9090:9090 -e PROCRELAY_PORT=9090 procrelay
```

```bash
# Docker Compose (edit compose.yml to set env vars):
docker compose up -d
```

Use `--help` for a full usage summary.

### API

Full schema: [`docs/openapi.yaml`](docs/openapi.yaml).

Base path: `/v1`. All responses are `application/json`.

```bash
# list all processes
curl http://localhost:8080/v1/processes

# filter by state
curl "http://localhost:8080/v1/processes?state=running"

# filter by time window (Unix epoch seconds, inclusive)
curl "http://localhost:8080/v1/processes?started_after=1718000000&started_before=1719000000"

# single process by PID
curl http://localhost:8080/v1/processes/1
```
