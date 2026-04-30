#pragma once
#include <string>
#include <vector>

namespace pc::proto {
struct Command {
    std::string name;
    std::vector<std::string> args;
};

Command parse_command(const std::string& line);
std::string ok();
std::string ok(const std::string& payload);
std::string err(int code, const std::string& message);
}
