// Pinterest-Clone Server - v0.1 echo skeleton
// BİL314 Bilgisayar Ağları Final Projesi

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr int  PORT          = 8080;
constexpr int  BACKLOG       = 32;
constexpr int  RECV_BUF_SIZE = 4096;

std::atomic<bool> g_running{true};

static void handle_client(int client_fd, sockaddr_in client_addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    std::cout << "[+] Client connected: " << ip << ":"
              << ntohs(client_addr.sin_port) << "\n";

    char buf[RECV_BUF_SIZE];
    while (g_running) {
        int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = '\0';
        std::cout << "[" << ip << "] -> " << buf;
        send(client_fd, buf, n, 0);
    }

    std::cout << "[-] Client disconnected: " << ip << "\n";
    close(client_fd);
}

int main() {
    int srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd < 0) {
        std::cerr << "socket() failed\n";
        return 1;
    }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    if (bind(srv_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed\n";
        return 1;
    }
    if (listen(srv_fd, BACKLOG) < 0) {
        std::cerr << "listen() failed\n";
        return 1;
    }

    std::cout << "Pinterest-Clone server v0.1 listening on :" << PORT << "\n";
    std::cout << "Press Ctrl+C to stop\n";

    std::vector<std::thread> threads;
    while (g_running) {
        sockaddr_in cli_addr{};
        socklen_t cli_len = sizeof(cli_addr);
        int cli_fd = accept(srv_fd, (sockaddr*)&cli_addr, &cli_len);
        if (cli_fd < 0) continue;
        threads.emplace_back(handle_client, cli_fd, cli_addr);
        threads.back().detach();
    }

    close(srv_fd);
    return 0;
}
