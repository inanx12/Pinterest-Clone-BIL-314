#pragma once
#include "db.hpp"
#include "session_cache.hpp"
#include <string>

namespace pc {

// Per-client thread'in entry point'i. Socket kapanana kadar komut dispatch eder.
// fd: client socket (call sonrası kapatılır)
// client_ip: log için
// db, cache: server-wide singleton'lar (paylaşılır)
void handle_client(int client_fd, const std::string& client_ip,
                   Database& db, SessionCache& cache);

}
