#include "db.hpp"
#include "server.hpp"
#include "session_cache.hpp"

#include <csignal>
#include <iostream>

namespace {

// Global pointer: signal handler ancak global'e ulasabilir
// (signal handler'a parametre gecirilemiyor).
pc::Server* g_server = nullptr;

void handle_sigint(int) {
    // Ctrl+C basildi -> server'i nazikce durdur.
    if (g_server) g_server->stop();
}

} // anon namespace

int main(int argc, char* argv[]) {
    // Port: argument verilmediyse 8080 (PROTOCOL.md default)
    int port = 8080;
    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Gecersiz port: " << argv[1] << "\n";
            return 1;
        }
    }

    // ----- Database -----
    pc::Database db("pcclone.db");
    if (!db.init()) {
        std::cerr << "DB initialize edilemedi.\n";
        return 1;
    }

    // ----- Session cache (in-memory) -----
    pc::SessionCache cache;

    // ----- Server -----
    pc::Server server(port, db, cache);
    g_server = &server;

    // Ctrl+C yakalama
    std::signal(SIGINT, handle_sigint);
    std::signal(SIGTERM, handle_sigint);

    // SIGPIPE ignore: client socket'i kapanmissa send() patlamasin
    // (ayrica MSG_NOSIGNAL kullaniyoruz session.cpp'de, double safety)
    std::signal(SIGPIPE, SIG_IGN);

    server.run();   // bloklu, Ctrl+C'ye kadar
    g_server = nullptr;
    return 0;
}
