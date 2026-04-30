// tests/test_main.cpp
// Manuel test: util + db + protocol modülleri.
// Build: cd build && cmake .. && make test_main && ./test_main

#include "util.hpp"
#include "db.hpp"
#include "protocol.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <set>
#include <sqlite3.h>

// Renkli çıktı (debug görsel)
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

    // Determinism
    auto h1 = pc::util::sha256_hex("hello");
    auto h2 = pc::util::sha256_hex("hello");
    CHECK(h1 == h2);

    // Bilinen vector: SHA256("hello") = 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824
    CHECK(h1 == "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");

    // Hash uzunluğu 64 hex char
    CHECK(h1.length() == 64);

    // Salt + password farklı hash üretmeli
    CHECK(pc::util::sha256_hex("hello") != pc::util::sha256_hex("hellox"));

    // random_hex — uzunluk doğru mu (16 byte = 32 hex char)
    auto r1 = pc::util::random_hex(16);
    auto r2 = pc::util::random_hex(16);
    CHECK(r1.length() == 32);
    CHECK(r2.length() == 32);

    // İki ardışık çağrı farklı sonuç vermeli (entropy testi)
    CHECK(r1 != r2);

    // 1000 üretimde hiç çakışma olmamalı
    std::set<std::string> seen;
    for (int i = 0; i < 1000; ++i) seen.insert(pc::util::random_hex(16));
    CHECK(seen.size() == 1000);

    // uuid_hex — 32 char
    CHECK(pc::util::uuid_hex().length() == 32);

    // now_unix — makul aralıkta (2026 ≈ 1.77 milyar)
    auto t = pc::util::now_unix();
    CHECK(t > 1700000000LL && t < 2000000000LL);
}

// ---------------------------------------------------------------- protocol
void test_protocol() {
    std::cout << "\n=== protocol ===\n";

    // Normal komut
    auto cmd = pc::proto::parse_command("LOGIN inan superpass\n");
    CHECK(cmd.name == "LOGIN");
    CHECK(cmd.args.size() == 2);
    CHECK(cmd.args[0] == "inan");
    CHECK(cmd.args[1] == "superpass");

    // Argümansız
    auto c2 = pc::proto::parse_command("LOGOUT\n");
    CHECK(c2.name == "LOGOUT");
    CHECK(c2.args.empty());

    // Boş satır
    auto c3 = pc::proto::parse_command("");
    CHECK(c3.name.empty());

    // CRLF (Qt client'tan gelebilir)
    auto c4 = pc::proto::parse_command("LIST tok newest 0 20\r\n");
    CHECK(c4.name == "LIST");
    CHECK(c4.args.size() == 4);
    CHECK(c4.args[3] == "20");   // \r tail temizlenmiş mi?

    // UPLOAD: çok argümanlı
    auto c5 = pc::proto::parse_command("UPLOAD abc image public foto.jpg 12345\n");
    CHECK(c5.name == "UPLOAD");
    CHECK(c5.args.size() == 5);
    CHECK(c5.args[4] == "12345");

    // Cevap formatlayıcılar
    CHECK(pc::proto::ok()                         == "OK\n");
    CHECK(pc::proto::ok("token123")               == "OK token123\n");
    CHECK(pc::proto::err(1001, "Invalid token")   == "ERR 1001 Invalid token\n");
    CHECK(pc::proto::err(1003, "Wrong password")  == "ERR 1003 Wrong password\n");
}

// ---------------------------------------------------------------- db
void test_db() {
    std::cout << "\n=== db ===\n";

    // Önceki testten kalan dosyayı temizle
    std::remove("test_pcclone.db");
    std::remove("test_pcclone.db-shm");
    std::remove("test_pcclone.db-wal");

    pc::Database db("test_pcclone.db");
    CHECK(db.init());

    // PRAGMA'lar gerçekten set olmuş mu? Raw sqlite3 ile kontrol.
    sqlite3* raw;
    sqlite3_open("test_pcclone.db", &raw);

    // foreign_keys ON mu? (Bu pragma connection-scoped — yeni connection açtığımız için
    // ON dönmeyebilir. Sadece WAL'ı kontrol edelim, o DB-wide kalıcı.)
    sqlite3_stmt* st;
    sqlite3_prepare_v2(raw, "PRAGMA journal_mode;", -1, &st, nullptr);
    sqlite3_step(st);
    std::string mode = (const char*)sqlite3_column_text(st, 0);
    CHECK(mode == "wal");
    sqlite3_finalize(st);

    // Beklenen tablolar var mı?
    const char* expected[] = {"users", "sessions", "media", "likes", "comments"};
    for (const char* tbl : expected) {
        std::string q = "SELECT name FROM sqlite_master WHERE type='table' AND name='";
        q += tbl;
        q += "';";
        sqlite3_prepare_v2(raw, q.c_str(), -1, &st, nullptr);
        bool found = (sqlite3_step(st) == SQLITE_ROW);
        sqlite3_finalize(st);
        std::cout << "  table " << tbl << ": " << (found ? "OK" : "MISSING") << "\n";
        CHECK(found);
    }

    // Beklenen indexler (en azından bizim oluşturduklarımız)
    const char* expected_idx[] = {
        "idx_sessions_user",
        "idx_media_owner",
        "idx_media_public_newest",
        "idx_comments_media"
    };
    for (const char* idx : expected_idx) {
        std::string q = "SELECT name FROM sqlite_master WHERE type='index' AND name='";
        q += idx;
        q += "';";
        sqlite3_prepare_v2(raw, q.c_str(), -1, &st, nullptr);
        bool found = (sqlite3_step(st) == SQLITE_ROW);
        sqlite3_finalize(st);
        CHECK(found);
    }

    sqlite3_close(raw);
}

// ----------------------------------------------------------------------
int main() {
    test_util();
    test_protocol();
    test_db();

    std::cout << "\n=========================\n";
    std::cout << "Passed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
