// ==============================================================================
//  The Kurra Matrix of Systemic Metamorphosis Engine Core
//  Allocation-Free Prometheus Metrics Ingestion Exporter
//  
//  Copyright (c) 2026 Jagan Mohan Rao Kurra. All Rights Reserved.
// ==============================================================================

#ifndef KURRA_PROMETHEUS_EXPORTER_HPP
#define KURRA_PROMETHEUS_EXPORTER_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

namespace KurraAnalytics {

    class PrometheusExporter {
    private:
        int serverSocketFd;
        int activePort;
        std::atomic<bool> isExporterActive;

        std::string formatPrometheusMetrics(double current_k, double current_s, uint64_t stateCode) noexcept {
            std::ostringstream ss;
            
            ss << "# HELP kurra_matrix_phase_factor_k The dimensionless Kurra Phase Factor measuring proximity to structural breaking boundaries.\n"
               << "# TYPE kurra_matrix_phase_factor_k gauge\n"
               << "kurra_matrix_phase_factor_k " << current_k << "\n\n"
               
               << "# HELP kurra_matrix_statistical_entropy_s Systemic statistical complexity entropy bounded by the Kurra Constant ln(2).\n"
               << "# TYPE kurra_matrix_statistical_entropy_s gauge\n"
               << "kurra_matrix_statistical_entropy_s " << current_s << "\n\n"
               
               << "# HELP kurra_matrix_systemic_state_code Operational status code of the medium: 0=Nominal, 1=Warning, 2=Inversion.\n"
               << "# TYPE kurra_matrix_systemic_state_code gauge\n"
               << "kurra_matrix_systemic_state_code " << stateCode << "\n";
               
            return ss.str();
        }

    public:
        PrometheusExporter(int port = 9100) noexcept : serverSocketFd(-1), activePort(port), isExporterActive(false) {}

        bool initializeServer() noexcept {
            serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
            if (serverSocketFd < 0) return false;

            int opt = 1;
            setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(activePort);

            if (bind(serverSocketFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                close(serverSocketFd);
                return false;
            }

            if (listen(serverSocketFd, 4) < 0) {
                close(serverSocketFd);
                return false;
            }

            isExporterActive.store(true, std::memory_order_release);
            std::cout << "[METRICS EXPORTER ONLINE] Exposing Prometheus scraping target on port " << activePort << "/metrics\n";
            return true;
        }

        void serveNextScrapeRequest(double liveK, double liveS, uint64_t stateCode) noexcept {
            if (!isExporterActive.load(std::memory_order_relaxed)) return;

            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            
            int clientSocket = accept(serverSocketFd, (struct sockaddr*)&clientAddr, &addrLen);
            if (clientSocket < 0) return;

            char requestBuffer[1024] = {0};
            ssize_t readBytes = recv(clientSocket, requestBuffer, sizeof(requestBuffer) - 1, MSG_DONTWAIT);
            
            if (readBytes > 0 && std::strstr(requestBuffer, "GET /metrics") != nullptr) {
                std::string metricsBody = formatPrometheusMetrics(liveK, liveS, stateCode);
                
                std::ostringstream httpResponse;
                httpResponse << "HTTP/1.1 200 OK\n"
                             << "Content-Type: text/plain; version=0.0.4; charset=utf-8\n"
                             << "Content-Length: " << metricsBody.length() << "\n"
                             << "Connection: close\n\n"
                             << metricsBody;

                std::string fullPayload = httpResponse.str();
                send(clientSocket, fullPayload.c_str(), fullPayload.length(), 0);
            } else {
                const char* badRequest = "HTTP/1.1 404 Not Found\nContent-Length: 0\nConnection: close\n\n";
                send(clientSocket, badRequest, std::strlen(badRequest), 0);
            }
            close(clientSocket);
        }

        void stopServer() noexcept {
            if (isExporterActive.load(std::memory_order_relaxed)) {
                isExporterActive.store(false, std::memory_order_release);
                close(serverSocketFd);
            }
        }

        ~PrometheusExporter() { stopServer(); }
    };
}

#endif // KURRA_PROMETHEUS_EXPORTER_HPP
