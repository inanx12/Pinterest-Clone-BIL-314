#pragma once
#include <string>
#include "db.hpp"

namespace pc {
void handle_client(int client_fd, const std::string& client_ip, Database& db);
}
