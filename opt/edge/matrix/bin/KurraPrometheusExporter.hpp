// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Strictly Non-Blocking, Zero-Heap-Allocation Prometheus Metrics Exporter
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
//  Licensed under the Apache License, Version 2.0
// ==============================================================================

#ifndef KURRA_PROMETHEUS_EXPORTER_HPP
#define KURRA_PROMETHEUS_EXPORTER_HPP

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <array>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

namespace KurraAnalytics {

    class PrometheusExporter {
    private:
        int serverSocketFd;
        int activePort;
        std::atomic<bool> isExporterActive;

    public:
        PrometheusExporter(int port = 9100) noexcept
            : serverSocketFd(-1), activePort(port), isExporterActive(false) {}

        bool initializeServer() noexcept {
            serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
            if (serverSocketFd < 0) return false;

            int flags = fcntl(serverSocketFd, F_GETFL, 0);
            if (flags < 0 || fcntl(serverSocketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
                close(serverSocketFd);
                return false;
            }

            int opt = 1;
            setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(activePort);

            if (bind(serverSocketFd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
                close(serverSocketFd);
                return false;
            }

            if (listen(serverSocketFd, 8) < 0) {
                close(serverSocketFd);
                return false;
            }

            isExporterActive.store(true, std::memory_order_release);
            return true;
        }

        void pollScrapeRequests(double liveK, double liveS, uint64_t stateCode) noexcept {
            if (!isExporterActive.load(std::memory_order_relaxed)) return;

            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);

            int clientSocket = accept(serverSocketFd, reinterpret_cast<struct sockaddr*>(&clientAddr), &addrLen);
            if (clientSocket < 0) return;

            std::array<char, 256> requestBuffer{};
            ssize_t readBytes = recv(clientSocket, requestBuffer.data(), requestBuffer.size() - 1, MSG_DONTWAIT);

            if (readBytes > 0 && std::strstr(requestBuffer.data(), "GET /metrics") != nullptr) {
                std::array<char, 512> bodyBuffer{};
                int bodyLen = std::snprintf(
                    bodyBuffer.data(), bodyBuffer.size(),
                    "# HELP kurra_matrix_phase_factor_k Dimensionless Kurra Phase Factor\n"
                    "# TYPE kurra_matrix_phase_factor_k gauge\n"
                    "kurra_matrix_phase_factor_k %.6f\n"
                    "# HELP kurra_matrix_statistical_entropy_s Bounded Shannon Entropy Surrogate\n"
                    "# TYPE kurra_matrix_statistical_entropy_s gauge\n"
                    "kurra_matrix_statistical_entropy_s %.6f\n"
                    "# HELP kurra_matrix_systemic_state_code State: 0=Nominal, 1=Warning, 2=Inversion\n"
                    "# TYPE kurra_matrix_systemic_state_code gauge\n"
                    "kurra_matrix_systemic_state_code %lu\n",
                    liveK, liveS, static_cast<unsigned long>(stateCode)
                );

                if (bodyLen > 0 && bodyLen < static_cast<int>(bodyBuffer.size())) {
                    std::array<char, 768> responseBuffer{};
                    int totalLen = std::snprintf(
                        responseBuffer.data(), responseBuffer.size(),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain; version=0.0.4; charset=utf-8\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n%s",
                        bodyLen, bodyBuffer.data()
                    );
                    if (totalLen > 0 && totalLen < static_cast<int>(responseBuffer.size())) {
                        send(clientSocket, responseBuffer.data(), static_cast<size_t>(totalLen), MSG_NOSIGNAL);
                    }
                }
            } else {
                const char* badRequest = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                send(clientSocket, badRequest, std::strlen(badRequest), MSG_NOSIGNAL);
            }

            close(clientSocket);
        }

        void stopServer() noexcept {
            if (isExporterActive.exchange(false, std::memory_order_acq_rel)) {
                if (serverSocketFd >= 0) close(serverSocketFd);
            }
        }

        ~PrometheusExporter() { stopServer(); }
    };
}

#endif // KURRA_PROMETHEUS_EXPORTER_HPP
