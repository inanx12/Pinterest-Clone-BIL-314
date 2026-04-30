#pragma once
#include <string>

namespace pc::util {
std::string sha256_hex(const std::string& input);
std::string random_hex(int byte_count);
std::string uuid_hex();
long long now_unix();
}
