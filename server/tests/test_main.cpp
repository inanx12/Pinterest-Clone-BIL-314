// tests/test_main.cpp
// Manuel test: util + db + protocol + auth modülleri.
// Build: g++ -std=c++17 -Isrc tests/test_main.cpp src/*.cpp -lsqlite3 -lcrypto -o build/test_main

#include "auth.hpp"
#include "db.hpp"
#include "protocol.hpp"
#include "session_cache.hpp"
#include "util.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <set>
#include <sqlite3.h>

#define GREEN "\033[32m"
#define RED   "\033[31m"
#define RESET "\033[0m"

static int passed = 0;
static int failed = 0;

#define CHECK(cond) do { \
    if (cond) { passed++; std::cout << GREEN << "  [OK]  " << RESET << #cond << "\n"; } \
    else      { failed++; std::cout << RED   << "  [FAIL]" << RESET << #cond << "\n"; } \
} while(0)

// ---------------------------------------------------------------- util
void test_util() {
    std::cout << "\n=== util ===\n";
    auto h1 = pc::util::sha256_hex("hello");
    auto h2 = pc::util::sha256_hex("hello");
    CHECK(h1 == h2);
    CHECK(h1 == "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
    CHECK(h1.length() == 64);
    CHECK(pc::util::sha256_hex("hello") != pc::util::sha256_hex("hellox"));

    auto r1 = pc::util::random_hex(16);
    auto r2 = pc::util::random_hex(16);
    CHECK(r1.length() == 32);
    CHECK(r2.length() == 32);
    CHECK(r1 != r2);

    std::set<std::string> seen;
    for (int i = 0; i < 1000; ++i) seen.insert(pc::util::random_hex(16));
    CHECK(seen.size() == 1000);

    CHECK(pc::util::uuid_hex().length() == 32);
    auto t = pc::util::now_unix();
    CHECK(t > 1700000000LL && t < 2000000000LL);
}

// ---------------------------------------------------------------- protocol
void test_protocol() {
    std::cout << "\n=== protocol ===\n";

    auto cmd = pc::proto::parse_command("LOGIN inan superpass\n");
    CHECK(cmd.name == "LOGIN");
    CHECK(cmd.args.size() == 2);
    CHECK(cmd.args[0] == "inan");
    CHECK(cmd.args[1] == "superpass");

    auto c2 = pc::proto::parse_command("LOGOUT\n");
    CHECK(c2.name == "LOGOUT");
    CHECK(c2.args.empty());

    auto c3 = pc::proto::parse_command("");
    CHECK(c3.name.empty());

    auto c4 = pc::proto::parse_command("LIST tok newest 0 20\r\n");
    CHECK(c4.name == "LIST");
    CHECK(c4.args.size() == 4);
    CHECK(c4.args[3] == "20");

    auto c5 = pc::proto::parse_command("UPLOAD abc image public foto.jpg 12345\n");
    CHECK(c5.name == "UPLOAD");
    CHECK(c5.args.size() == 5);
    CHECK(c5.args[4] == "12345");

    // Lowercase komut da uppercase olmali
    auto c6 = pc::proto::parse_command("login inan pass\n");
    CHECK(c6.name == "LOGIN");

    CHECK(pc::proto::ok()                         == "OK\n");
    CHECK(pc::proto::ok("token123")               == "OK token123\n");
    CHECK(pc::proto::err(1001, "Invalid token")   == "ERR 1001 Invalid token\n");
    CHECK(pc::proto::err(1003, "Wrong password")  == "ERR 1003 Wrong password\n");
}

// ---------------------------------------------------------------- db
void test_db() {
    std::cout << "\n=== db ===\n";

    std::remove("test_pcclone.db");
    std::remove("test_pcclone.db-shm");
    std::remove("test_pcclone.db-wal");

    pc::Database db("test_pcclone.db");
    CHECK(db.init());

    sqlite3* raw;
    sqlite3_open("test_pcclone.db", &raw);

    sqlite3_stmt* st;
    sqlite3_prepare_v2(raw, "PRAGMA journal_mode;", -1, &st, nullptr);
    sqlite3_step(st);
    std::string mode = (const char*)sqlite3_column_text(st, 0);
    CHECK(mode == "wal");
    sqlite3_finalize(st);

    const char* expected[] = {"users", "sessions", "media", "likes", "comments"};
    for (const char* tbl : expected) {
        std::string q = std::string("SELECT name FROM sqlite_master WHERE type='table' AND name='") + tbl + "';";
        sqlite3_prepare_v2(raw, q.c_str(), -1, &st, nullptr);
        bool found = (sqlite3_step(st) == SQLITE_ROW);
        sqlite3_finalize(st);
        std::cout << "  table " << tbl << ": " << (found ? "OK" : "MISSING") << "\n";
        CHECK(found);
    }

    const char* expected_idx[] = {
        "idx_sessions_user", "idx_media_owner",
        "idx_media_public_newest", "idx_comments_media"
    };
    for (const char* idx : expected_idx) {
        std::string q = std::string("SELECT name FROM sqlite_master WHERE type='index' AND name='") + idx + "';";
        sqlite3_prepare_v2(raw, q.c_str(), -1, &st, nullptr);
        bool found = (sqlite3_step(st) == SQLITE_ROW);
        sqlite3_finalize(st);
        CHECK(found);
    }

    sqlite3_close(raw);
}

// ---------------------------------------------------------------- auth
void test_auth() {
    std::cout << "\n=== auth ===\n";

    std::remove("test_auth.db");
    std::remove("test_auth.db-shm");
    std::remove("test_auth.db-wal");

    pc::Database db("test_auth.db");
    CHECK(db.init());
    pc::SessionCache cache;

    // REGISTER: happy path
    auto r1 = pc::register_user(db, cache, "inan", "secret123");
    CHECK(r1.success);
    CHECK(r1.token.length() == 32);

    // REGISTER: ayni username tekrar -> 1002
    auto r2 = pc::register_user(db, cache, "inan", "another");
    CHECK(!r2.success);
    CHECK(r2.error_code == 1002);

    // REGISTER: case-insensitive collision (INAN da inan ile cakismali)
    auto r3 = pc::register_user(db, cache, "INAN", "another");
    CHECK(!r3.success);
    CHECK(r3.error_code == 1002);

    // REGISTER: kisa username -> 1006
    auto r4 = pc::register_user(db, cache, "ab", "longpass");
    CHECK(!r4.success);
    CHECK(r4.error_code == 1006);

    // REGISTER: gecerli username
    auto r5 = pc::register_user(db, cache, "senol_h", "longpass");
    CHECK(r5.success);

    // REGISTER: bosluk iceren username -> 1006
    auto r5b = pc::register_user(db, cache, "ali bey", "longpass");
    CHECK(!r5b.success);
    CHECK(r5b.error_code == 1006);

    // REGISTER: kisa sifre -> 1006
    auto r6 = pc::register_user(db, cache, "newuser", "abc");
    CHECK(!r6.success);
    CHECK(r6.error_code == 1006);

    // LOGIN: dogru sifre -> yeni token (multi-device)
    auto l1 = pc::login_user(db, cache, "inan", "secret123");
    CHECK(l1.success);
    CHECK(l1.token != r1.token);

    // LOGIN: yanlis sifre -> 1003
    auto l2 = pc::login_user(db, cache, "inan", "wrong");
    CHECK(!l2.success);
    CHECK(l2.error_code == 1003);

    // LOGIN: olmayan kullanici -> 1003
    auto l3 = pc::login_user(db, cache, "ghost", "anything");
    CHECK(!l3.success);
    CHECK(l3.error_code == 1003);

    // LOGIN: case-insensitive username
    auto l4 = pc::login_user(db, cache, "INAN", "secret123");
    CHECK(l4.success);

    // VALIDATE_TOKEN: gecerli (cache hit)
    auto u1 = pc::validate_token(db, cache, r1.token);
    CHECK(u1.has_value());
    CHECK(u1->username == "inan");

    // VALIDATE_TOKEN: gecersiz
    auto u2 = pc::validate_token(db, cache, "fake_token_doesnt_exist");
    CHECK(!u2.has_value());

    // LOGOUT
    CHECK(pc::logout(db, cache, r1.token));
    auto u3 = pc::validate_token(db, cache, r1.token);
    CHECK(!u3.has_value());

    // Multi-device: l1 token hala gecerli (logout sadece o token'i siler)
    auto u4 = pc::validate_token(db, cache, l1.token);
    CHECK(u4.has_value());

    // Cache miss -> DB fallback (server restart simulasyonu)
    pc::SessionCache fresh_cache;
    auto u5 = pc::validate_token(db, fresh_cache, l1.token);
    CHECK(u5.has_value());
    CHECK(u5->username == "inan");

    // Ikinci cagri artik cache'ten gelmeli (write-through doldurdu)
    auto u6 = fresh_cache.get(l1.token);
    CHECK(u6.has_value());
}

// ----------------------------------------------------------------------
int main() {
    test_util();
    test_protocol();
    test_db();
    test_auth();

    std::cout << "\n=========================\n";
    std::cout << "Passed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
