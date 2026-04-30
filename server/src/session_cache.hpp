#pragma once
#include "auth.hpp"  // User struct için
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace pc {

// Thread-safe in-memory session cache.
// Login/Register sonrası token burada tutulur, sonraki komutlarda DB sorgusu yapılmaz.
// Cache miss = server restart sonrası senaryo → DB'den okunup tekrar doldurulur.
class SessionCache {
public:
    void put(const std::string& token, const User& user);
    std::optional<User> get(const std::string& token);
    void remove(const std::string& token);

    // CHANGE_PASSWORD'da kullanılacak: bir kullanıcının tüm aktif tokenlarını sil.
    void remove_all_for_user(int user_id);

private:
    std::mutex mu_;
    std::unordered_map<std::string, User> map_;
};

}
