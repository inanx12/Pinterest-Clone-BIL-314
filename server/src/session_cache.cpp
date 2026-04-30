#include "session_cache.hpp"

namespace pc {

void SessionCache::put(const std::string& token, const User& user) {
    std::lock_guard<std::mutex> lock(mu_);
    map_[token] = user;
}

std::optional<User> SessionCache::get(const std::string& token) {
    std::lock_guard<std::mutex> lock(mu_);
    auto it = map_.find(token);
    if (it == map_.end()) return std::nullopt;
    return it->second;
}

void SessionCache::remove(const std::string& token) {
    std::lock_guard<std::mutex> lock(mu_);
    map_.erase(token);
}

void SessionCache::remove_all_for_user(int user_id) {
    std::lock_guard<std::mutex> lock(mu_);
    for (auto it = map_.begin(); it != map_.end();) {
        if (it->second.id == user_id) it = map_.erase(it);
        else ++it;
    }
}

}
