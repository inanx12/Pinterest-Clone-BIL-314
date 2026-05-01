#pragma once
#include "db.hpp"
#include "session_cache.hpp"
#include <atomic>

namespace pc {

// TCP server: belirtilen port'ta dinler, gelen her client için
// ayrı bir thread'de handle_client çalıştırır.
class Server {
public:
    Server(int port, Database& db, SessionCache& cache);
    ~Server();

    // Kopyalanamaz/taşınamaz (socket fd'si tek olmalı).
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Sonsuz accept loop — sadece stop() çağrılınca veya hata olunca döner.
    void run();

    // Başka thread'den çağrılabilir (örn. SIGINT handler).
    // running_ flag'ini false yapar, listen socket'i kapatır.
    void stop();

private:
    int port_;
    int listen_fd_ = -1;
    Database& db_;
    SessionCache& cache_;
    std::atomic<bool> running_{false};
};

}
