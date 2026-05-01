#include "server.hpp"
#include "session.hpp"

#include <arpa/inet.h>      // inet_ntop
#include <cerrno>
#include <cstring>          // strerror
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>         // close

namespace pc {

Server::Server(int port, Database& db, SessionCache& cache)
    : port_(port), db_(db), cache_(cache) {}

Server::~Server() {
    stop();
}

void Server::run() {
    // ----- 1. Listen socket oluştur -----
    // AF_INET = IPv4, SOCK_STREAM = TCP, 0 = default protocol
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        std::cerr << "socket() hatasi: " << std::strerror(errno) << "\n";
        return;
    }

    // ----- 2. SO_REUSEADDR: restart'ta "Address already in use" engelle -----
    int opt = 1;
    if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt() hatasi: " << std::strerror(errno) << "\n";
        // Fatal degil, devam edebiliriz.
    }

    // ----- 3. Adresi bind et -----
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // tum interface'lerde dinle
    addr.sin_port        = htons(port_);        // host -> network byte order

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind() hatasi (port " << port_ << "): "
                  << std::strerror(errno) << "\n";
        ::close(listen_fd_);
        listen_fd_ = -1;
        return;
    }

    // ----- 4. Dinlemeye baslat -----
    // 128 = backlog: accept edilmeden once kuyrukta bekleyebilecek baglanti sayisi
    if (::listen(listen_fd_, 128) < 0) {
        std::cerr << "listen() hatasi: " << std::strerror(errno) << "\n";
        ::close(listen_fd_);
        listen_fd_ = -1;
        return;
    }

    running_ = true;
    std::cout << "Server dinliyor: port " << port_ << "\n";

    // ----- 5. Accept loop -----
    while (running_) {
        sockaddr_in client_addr{};
        socklen_t   client_len = sizeof(client_addr);

        int client_fd = ::accept(listen_fd_,
                                 reinterpret_cast<sockaddr*>(&client_addr),
                                 &client_len);
        if (client_fd < 0) {
            // stop() listen_fd_'yi kapatmis olabilir -> accept EBADF/EINVAL doner.
            // Bu beklenen bir cikis, hata basma.
            if (!running_) break;
            std::cerr << "accept() hatasi: " << std::strerror(errno) << "\n";
            continue;
        }

        // Client IP'sini string'e cevir (log icin)
        char ip_buf[INET_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &client_addr.sin_addr, ip_buf, sizeof(ip_buf));
        std::string client_ip = ip_buf;

        std::cout << "[" << client_ip << "] baglandi (fd=" << client_fd << ")\n";

        // ----- 6. Yeni thread'de handle_client'i calistir -----
        // detach: ana thread bunu beklemiyor, kendi kendine bitsin.
        std::thread([client_fd, client_ip, this]() {
            handle_client(client_fd, client_ip, db_, cache_);
        }).detach();
    }

    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
    std::cout << "Server kapandi.\n";
}

void Server::stop() {
    running_ = false;
    if (listen_fd_ >= 0) {
        // shutdown() + close(): accept() bloklu beklemekten kurtulsun.
        ::shutdown(listen_fd_, SHUT_RDWR);
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
}

}
