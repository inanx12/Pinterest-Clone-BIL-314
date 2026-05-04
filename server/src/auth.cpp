#include "auth.hpp"
#include "session_cache.hpp"
#include "util.hpp"
#include <cctype>
#include <sqlite3.h>

namespace pc {

namespace {

// Protokol §3: 3-20 karakter, sadece [a-zA-Z0-9_].
bool valid_username(const std::string& u) {
    if (u.size() < 3 || u.size() > 20) return false;
    for (unsigned char c : u) {
        if (!std::isalnum(c) && c != '_') return false;
    }
    return true;
}

// Protokol §3: minimum 6 karakter.
bool valid_password(const std::string& p) {
    return p.size() >= 6;
}

// Yeni session oluştur: token üret → DB'ye yaz → cache'e koy.
// Başarısızsa boş string döner.
std::string create_session(Database& db, SessionCache& cache,
                           int user_id, const std::string& username) {
    std::string token = util::random_hex(16); // 32 hex char

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(),
            "INSERT INTO sessions (token, user_id, created_at) VALUES (?, ?, ?);",
            -1, &stmt, nullptr) != SQLITE_OK) {
        return "";
    }
    sqlite3_bind_text (stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int  (stmt, 2, user_id);
    sqlite3_bind_int64(stmt, 3, util::now_unix());

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) return "";

    cache.put(token, User{user_id, username});
    return token;
}

} // anon namespace

// =========================================================================
RegisterResult register_user(Database& db, SessionCache& cache,
                             const std::string& username,
                             const std::string& password) {
    RegisterResult r{false, "", "", 0};

    if (!valid_username(username) || !valid_password(password)) {
        r.error = "Invalid username or password";
        r.error_code = 1006;
        return r;
    }

    // Username zaten var mı? (UNIQUE constraint zaten kırar ama düzgün hata kodu için manuel kontrol.)
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.handle(),
        "SELECT 1 FROM users WHERE username = ? COLLATE NOCASE;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    if (exists) {
        r.error = "Username taken";
        r.error_code = 1002;
        return r;
    }

    // Salt + hash
    std::string salt = util::random_hex(16);
    if (salt.empty()) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }
    std::string hash = util::sha256_hex(password + salt);

    // INSERT user
    sqlite3_prepare_v2(db.handle(),
        "INSERT INTO users (username, password_hash, salt, created_at) VALUES (?, ?, ?, ?);",
        -1, &stmt, nullptr);
    sqlite3_bind_text (stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 2, hash.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 3, salt.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, util::now_unix());
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }

    int user_id = static_cast<int>(sqlite3_last_insert_rowid(db.handle()));

    // İlk session
    std::string token = create_session(db, cache, user_id, username);
    if (token.empty()) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }

    r.success = true;
    r.token = token;
    return r;
}

// =========================================================================
LoginResult login_user(Database& db, SessionCache& cache,
                       const std::string& username,
                       const std::string& password) {
    LoginResult r{false, "", "", 0};

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.handle(),
        "SELECT id, username, password_hash, salt FROM users WHERE username = ? COLLATE NOCASE;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        // Username yok — ama timing leak olmasın diye yine de "wrong username or password" diyoruz.
        r.error = "Wrong username or password";
        r.error_code = 1003;
        return r;
    }

    int id = sqlite3_column_int(stmt, 0);
    std::string actual_username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    std::string stored_hash     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    std::string salt            = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    sqlite3_finalize(stmt);

    std::string computed = util::sha256_hex(password + salt);
    if (computed != stored_hash) {
        r.error = "Wrong username or password";
        r.error_code = 1003;
        return r;
    }

    std::string token = create_session(db, cache, id, actual_username);
    if (token.empty()) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }

    r.success = true;
    r.token = token;
    return r;
}

// =========================================================================
std::optional<User> validate_token(Database& db, SessionCache& cache,
                                   const std::string& token) {
    // Hızlı yol: cache.
    auto cached = cache.get(token);
    if (cached) return cached;

    // Yavaş yol: DB lookup (server restart sonrası ilk kullanım).
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.handle(),
        "SELECT u.id, u.username FROM sessions s "
        "JOIN users u ON u.id = s.user_id WHERE s.token = ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);

    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cache.put(token, u);  // bir daha cache miss olmasın
        result = u;
    }
    sqlite3_finalize(stmt);
    return result;
}

// =========================================================================
bool logout(Database& db, SessionCache& cache, const std::string& token) {
    cache.remove(token);

    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db.handle(),
        "DELETE FROM sessions WHERE token = ?;",
        -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Var olmayan token logout'u bile OK döner (idempotent).
    return rc == SQLITE_DONE;
}

// =========================================================================
// CHANGE_PASSWORD
// =========================================================================
// Sıra (önemli):
//   1. Token validate → user_id
//   2. Old password kontrol (mevcut salt + hash)
//   3. New password validate (>=6)
//   4. Yeni salt + yeni hash UPDATE users
//   5. DELETE FROM sessions WHERE user_id=?  (multi-device dahil hepsi)
//   6. cache.remove_all_for_user(user_id)
//   7. Yeni session create → new_token
//
// Eski salt'ı kullanan tüm tokenlar uçar. Bu cihaz dahil.
// Cevap olarak yeni token döndürüyoruz (protokol §4.11).
ChangePasswordResult change_password(Database& db, SessionCache& cache,
                                     const std::string& token,
                                     const std::string& old_password,
                                     const std::string& new_password) {
    ChangePasswordResult r{false, "", "", 0};

    // ----- 1) Token validate, user bilgisini al -----
    auto user_opt = validate_token(db, cache, token);
    if (!user_opt) {
        r.error = "Invalid or expired token";
        r.error_code = 1001;
        return r;
    }
    int user_id = user_opt->id;
    std::string username = user_opt->username;

    // ----- 2) Mevcut salt + hash'i çek, old_password kontrol -----
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(),
            "SELECT password_hash, salt FROM users WHERE id = ?;",
            -1, &stmt, nullptr) != SQLITE_OK) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        // Token validate olmuştu ama user yok → kayıt silinmiş, çok nadir
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }
    std::string stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    std::string old_salt    = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    sqlite3_finalize(stmt);

    if (util::sha256_hex(old_password + old_salt) != stored_hash) {
        r.error = "Wrong password";
        r.error_code = 1003;
        return r;
    }

    // ----- 3) Yeni şifreyi validate et -----
    if (new_password.size() < 6) {
        r.error = "New password too short";
        r.error_code = 1006;
        return r;
    }

    // ----- 4) Yeni salt + hash, UPDATE users -----
    std::string new_salt = util::random_hex(16);
    if (new_salt.empty()) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }
    std::string new_hash = util::sha256_hex(new_password + new_salt);

    if (sqlite3_prepare_v2(db.handle(),
            "UPDATE users SET password_hash = ?, salt = ? WHERE id = ?;",
            -1, &stmt, nullptr) != SQLITE_OK) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }
    sqlite3_bind_text(stmt, 1, new_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, new_salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 3, user_id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }

    // ----- 5) Tüm eski sessions'ı DB'den sil -----
    if (sqlite3_prepare_v2(db.handle(),
            "DELETE FROM sessions WHERE user_id = ?;",
            -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_step(stmt);  // hata olsa bile devam — cache temizliği yine yapılacak
        sqlite3_finalize(stmt);
    }

    // ----- 6) Cache'ten de bu kullanıcının tüm tokenlarını sil -----
    cache.remove_all_for_user(user_id);

    // ----- 7) Yeni session oluştur -----
    std::string new_token = create_session(db, cache, user_id, username);
    if (new_token.empty()) {
        r.error = "Internal error";
        r.error_code = 2002;
        return r;
    }

    r.success = true;
    r.new_token = new_token;
    return r;
}

}
