// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Hardened Linux Edge Daemon Processor (Fixed argv & Memory Locking)
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
//  Licensed under the Apache License, Version 2.0
// ==============================================================================

#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstring>

#ifdef __linux__
#include <sys/mman.h>
#endif

std::atomic<bool> daemonActive{true};

void handleSignal(int signal) {
    if (signal == SIGTERM || signal == SIGINT) {
        daemonActive.store(false, std::memory_order_release);
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGTERM, handleSignal);
    std::signal(SIGINT, handleSignal);

#ifdef __linux__
    // Lock current and future pages into physical RAM to prevent swap jitter
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        std::cerr << "[WARNING] mlockall failed (run with CAP_IPC_LOCK or root for hard real-time).\n";
    }
#endif

    // Safely check argv[1] instead of comparing the argv pointer array
    bool isMaster = (argc > 1 && std::strcmp(argv[1], "--master") == 0);

    std::cout << "[DAEMON STARTED] Kurra Matrix Edge Node running in "
              << (isMaster ? "MASTER" : "WORKER") << " configuration.\n";

    while (daemonActive.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "[DAEMON SHUTDOWN] Matrix service terminated cleanly.\n";
    return 0;
}
