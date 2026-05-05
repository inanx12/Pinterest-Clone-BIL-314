#pragma once
#include "db.hpp"
#include <optional>
#include <string>

namespace pc {

class SessionCache;  // forward decl — gerçek tanım session_cache.hpp'de

struct User {
    int id;
    std::string username;
};

struct RegisterResult {
    bool success;
    std::string token;
    std::string error;
    int error_code;
};

struct LoginResult {
    bool success;
    std::string token;
    std::string error;
    int error_code;
};


// REGISTER — yeni kullanıcı + ilk session oluştur, token döndür.
RegisterResult register_user(Database& db, SessionCache& cache,
                             const std::string& username,
                             const std::string& password);

// LOGIN — şifre doğrula, yeni session oluştur (multi-device: önceki tokenlar dokunulmaz).
LoginResult login_user(Database& db, SessionCache& cache,
                       const std::string& username,
                       const std::string& password);

// Token geçerli mi? Önce cache, miss olursa DB fallback.
std::optional<User> validate_token(Database& db, SessionCache& cache,
                                   const std::string& token);

// Session'ı hem cache'ten hem DB'den sil.
bool logout(Database& db, SessionCache& cache, const std::string& token);

// =========================================================================
// CHANGE_PASSWORD
// =========================================================================
// Akış: validate token → old_password kontrol → new_password validate
//       → yeni salt+hash UPDATE → tüm sessions DELETE → cache flush
//       → yeni session create → new_token döndür.
//
// Tüm eski tokenlar invalidate olur (multi-device dahil) — kullanıcı
// diğer cihazlardan yeniden login olmak zorunda.
struct ChangePasswordResult {
    bool success;
    std::string new_token;
    std::string error;
    int error_code;
};

ChangePasswordResult change_password(Database& db, SessionCache& cache,
                                     const std::string& token,
                                     const std::string& old_password,
                                     const std::string& new_password);

}
