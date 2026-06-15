# High-Performance C++ Web Server Engine

[![C++ Core Build](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/YOUR_REPO/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight, multithreaded HTTP web server engineered from scratch in C++17. Designed with an Nginx-style architecture, this engine utilizes native POSIX sockets, a custom thread pool, and non-blocking I/O to safely serve static assets at high concurrency.

## Core Architecture

* **Custom Thread Pool:** Pre-allocates worker threads to handle incoming TCP connections concurrently without the overhead of spinning up new threads per request.
* **Zero-Dependency HTTP Parser:** Dynamically extracts requested routing paths and determines accurate MIME types without relying on external libraries.
* **Robust File I/O:** Safely streams binary and text files (HTML, CSS, JS, PNG, JPG) directly from the local filesystem to the network layer.
* **Graceful Shutdown Integration:** Intercepts native Linux signals (`SIGINT`, `SIGTERM`) to cleanly finish active requests and prevent memory corruption.
* **Thread-Safe Logging:** Utilizes `std::mutex` locking to prevent console interleaving and provide professional, timestamped access logs.

## Build & Installation

This engine is built natively for Linux environments and utilizes CMake for cross-platform build generation.

### Prerequisites
* CMake (3.15 or higher)
* A modern C++ compiler (g++ or clang) supporting C++17

### Compilation
```bash
git clone [https://github.com/aswinganga-greenink/custom-webserver.git](https://github.com/aswinganga-greenink/custom-webserver.git)
cd custom-webserver

# Generate the build system
cmake -B build -DENABLE_ASAN=OFF

# Compile the engine
cmake --build build