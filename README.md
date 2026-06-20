# Raven Engine

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-success.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![Docker](https://img.shields.io/badge/docker-ready-2496ED.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**Raven Engine** is a high-performance, asynchronous HTTP server engineered from the ground up for native Linux environments. Built in C++17, it utilizes the POSIX `epoll` API and a custom thread-pool architecture to bypass blocking I/O, allowing it to handle concurrent client connections with near-zero latency.

## Features

* **Event-Driven Architecture:** Utilizes edge-triggered `epoll` to manage thousands of concurrent connections on a single thread.
* **Asynchronous Thread Pool:** Incoming HTTP requests are dynamically dispatched to pre-allocated worker threads, preventing CPU blocking.
* **Keep-Alive Reaper:** A dedicated background daemon manages a priority queue to gracefully drop stale TCP connections.
* **Zero-Downtime Reboots:** Implements `SO_REUSEADDR` to prevent OS-level port locking during rapid deployments.
* **Mathematical Path Armor:** Leverages `std::filesystem::weakly_canonical` to mathematically block malicious directory traversal (e.g., `../`) attacks.
* **Macro-Driven Telemetry:** A high-performance logging engine that evaluates verbosity thresholds at compile-time to prevent string-allocation overhead in production.

---

## Architecture Overview

Unlike traditional thread-per-connection web servers (like early Apache), Raven Engine mimics the architecture of Nginx:
1. **The Event Loop:** The main thread sits entirely within `epoll_wait`, monitoring non-blocking sockets.
2. **The Handoff:** Upon receiving an `EPOLLIN` event, the main thread reads the raw HTTP bytes and hands the payload to the Thread Pool.
3. **The Workers:** Worker threads parse the HTTP request, resolve the file path, construct the MIME types, and stream the binary response back to the client.

---

## Getting Started

### Prerequisites
* **OS:** Linux (Ubuntu/Debian recommended)
* **Compiler:** GCC 9+ or Clang 10+ (Must support C++17)
* **Tools:** CMake 3.10+, Make
* **Optional:** Docker & Docker Compose

### Deployment via Docker (Recommended)
Raven Engine is packaged as a multi-stage, immutable Docker container. For active development, use volume mounts to enable live-reloading of HTML/CSS assets without recompiling the binary.

```bash
# 1. Build the lightweight image
docker build -t raven-engine:v1 .

# 2. Run with live volume mounts
docker run -d \
  --name raven-server \
  -p 8080:8080 \
  -v $(pwd)/server.conf:/app/server.conf \
  -v $(pwd)/public:/app/public \
  raven-engine:v1