#pragma once
#include <atomic>
#include "db.hpp"

namespace pc {
class Server {
public:
    Server(int port, Database& db);
    ~Server();
    
    void run();
    void stop();
    
private:
    int port_;
    int listen_fd_ = -1;
    Database& db_;
    std::atomic<bool> running_{false};
};
}
