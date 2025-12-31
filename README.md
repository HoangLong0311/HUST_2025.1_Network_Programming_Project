# P2P File Sharing System

A robust Peer-to-Peer (P2P) file sharing system implemented in C using TCP Sockets. This system employs a **Hybrid Architecture** with a central Server for peer discovery and direct Peer-to-Peer connections for file transfer.

## Features

* **Centralized Server:** Manages active peers and their shared files.
* **P2P File Transfer:** Direct file download between clients .

## Project Structure

```
.
├── common/             # Shared headers and logic (protocol definitions)
├── server/
│   ├── src/            # Server implementation
│   └── include/        # Server headers
├── client/
│   ├── src/            # Client implementation
│   └── include/        # Client headers
├── Makefile            # Build system
└── README.md

```

## Prerequisites
* OS: Linux (Recommend). Windows users need WSL or MinGW.

* Compiler: GCC.

* Build System: GNU Make.

## Usage Guide

### 1. Build & Clean

* **`make`**: Compiles both Server and Client source code. Generates executables in the `bin/` directory.
* **`make clean`**: Removes all build artifacts (`bin/`, `build/`) and cleans up the test environment directories (`test/`).

### 2. Running the Network

* **`make run-server`**
    * Starts the Server.
    * Default: Listens on Port **8080**.

* **`make run-client<N>`** (e.g., `make run-client1`, `make run-client2`)
    * Sets up the test environment at `test/client<N>`.
    * Automatically generate sample files in `shared`.
    
