// tests/test_main.cpp
// Manuel test: util + db + protocol + auth + session.
// Build: g++ -std=c++17 -Isrc tests/test_main.cpp src/*.cpp -lsqlite3 -lcrypto -pthread -o build/test_main

#include "auth.hpp"
#include "db.hpp"
#include "protocol.hpp"
#include "session.hpp"
#include "session_cache.hpp"
#include "util.hpp"

#include <cstdio>
#include <iostream>
#include <set>
#include <sqlite3.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

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
    CHECK(h1 == "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
    CHECK(h1.length() == 64);
    CHECK(pc::util::sha256_hex("hello") != pc::util::sha256_hex("hellox"));

    auto r1 = pc::util::random_hex(16);
    auto r2 = pc::util::random_hex(16);
    CHECK(r1.length() == 32);
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

    auto c4 = pc::proto::parse_command("LIST tok newest 0 20\r\n");
    CHECK(c4.args[3] == "20");

    auto c6 = pc::proto::parse_command("login inan pass\n");
    CHECK(c6.name == "LOGIN");

    CHECK(pc::proto::ok()                     == "OK\n");
    CHECK(pc::proto::ok("token123")           == "OK token123\n");
    CHECK(pc::proto::err(1001, "Bad token")   == "ERR 1001 Bad token\n");
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

    auto r1 = pc::register_user(db, cache, "inan", "secret123");
    CHECK(r1.success);
    CHECK(r1.token.length() == 32);

    auto r2 = pc::register_user(db, cache, "inan", "another");
    CHECK(r2.error_code == 1002);

    auto r3 = pc::register_user(db, cache, "INAN", "another");
    CHECK(r3.error_code == 1002);

    auto r4 = pc::register_user(db, cache, "ab", "longpass");
    CHECK(r4.error_code == 1006);

    auto r5b = pc::register_user(db, cache, "ali bey", "longpass");
    CHECK(r5b.error_code == 1006);

    auto r6 = pc::register_user(db, cache, "newuser", "abc");
    CHECK(r6.error_code == 1006);

    auto l1 = pc::login_user(db, cache, "inan", "secret123");
    CHECK(l1.success);
    CHECK(l1.token != r1.token);

    auto l2 = pc::login_user(db, cache, "inan", "wrong");
    CHECK(l2.error_code == 1003);

    auto l3 = pc::login_user(db, cache, "ghost", "anything");
    CHECK(l3.error_code == 1003);

    auto l4 = pc::login_user(db, cache, "INAN", "secret123");
    CHECK(l4.success);

    auto u1 = pc::validate_token(db, cache, r1.token);
    CHECK(u1.has_value());
    CHECK(u1->username == "inan");

    auto u2 = pc::validate_token(db, cache, "fake_token_doesnt_exist");
    CHECK(!u2.has_value());

    CHECK(pc::logout(db, cache, r1.token));
    auto u3 = pc::validate_token(db, cache, r1.token);
    CHECK(!u3.has_value());

    auto u4 = pc::validate_token(db, cache, l1.token);
    CHECK(u4.has_value());

    pc::SessionCache fresh_cache;
    auto u5 = pc::validate_token(db, fresh_cache, l1.token);
    CHECK(u5.has_value());
    CHECK(u5->username == "inan");
}

// ---------------------------------------------------------------- session
//
// socketpair ile gerçek socket akışı simulasyonu:
// fds[0] = "client" tarafı (test bu uca yazıp/okuyor)
// fds[1] = "server" tarafı (handle_client bu uca veriliyor, kapatma sorumluluğu onun)

// fd'den \n'e kadar oku
static std::string recv_line(int fd) {
    std::string out;
    char c;
    while (recv(fd, &c, 1, 0) == 1) {
        if (c == '\n') break;
        out.push_back(c);
    }
    return out;
}

static void send_line(int fd, const std::string& s) {
    std::string with_nl = s + "\n";
    send(fd, with_nl.data(), with_nl.size(), 0);
}

void test_session() {
    std::cout << "\n=== session (dispatch) ===\n";
    std::remove("test_session.db");
    std::remove("test_session.db-shm");
    std::remove("test_session.db-wal");

    pc::Database db("test_session.db");
    CHECK(db.init());
    pc::SessionCache cache;

    // ----- Senaryo 1: REGISTER + LOGIN + LOGOUT happy path -----
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
        std::thread t([&]() {
            pc::handle_client(fds[1], "test", db, cache);
        });

        send_line(fds[0], "REGISTER inan superpass");
        std::string r = recv_line(fds[0]);
        CHECK(r.substr(0, 3) == "OK ");
        CHECK(r.size() == 35);  // "OK " + 32 hex char

        send_line(fds[0], "LOGIN inan superpass");
        std::string l = recv_line(fds[0]);
        CHECK(l.substr(0, 3) == "OK ");
        std::string second_token = l.substr(3);
        CHECK(second_token.size() == 32);

        send_line(fds[0], "LOGOUT " + second_token);
        std::string lo = recv_line(fds[0]);
        CHECK(lo == "OK");

        // Bağlantıyı kapat, thread bitsin
        close(fds[0]);
        t.join();
    }

    // ----- Senaryo 2: lowercase komut + persistent connection (multiple commands) -----
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
        std::thread t([&]() {
            pc::handle_client(fds[1], "test", db, cache);
        });

        // login lowercase, REGISTER zaten var (önceki senaryodan)
        send_line(fds[0], "login inan superpass");
        std::string l = recv_line(fds[0]);
        CHECK(l.substr(0, 3) == "OK ");

        // Aynı bağlantıda ikinci komut: yanlış şifreyle login
        send_line(fds[0], "LOGIN inan WRONG");
        std::string l2 = recv_line(fds[0]);
        CHECK(l2 == "ERR 1003 Wrong username or password");

        close(fds[0]);
        t.join();
    }

    // ----- Senaryo 3: pipelined komutlar (tek recv'de iki satır) -----
    // read_line buffer'ının doğru çalıştığını doğrular.
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
        std::thread t([&]() {
            pc::handle_client(fds[1], "test", db, cache);
        });

        std::string both = "REGISTER pipelined password1\nLOGIN pipelined password1\n";
        send(fds[0], both.data(), both.size(), 0);

        std::string r1 = recv_line(fds[0]);
        std::string r2 = recv_line(fds[0]);
        CHECK(r1.substr(0, 3) == "OK ");
        CHECK(r2.substr(0, 3) == "OK ");

        close(fds[0]);
        t.join();
    }

    // ----- Senaryo 4: hatalı argüman sayısı -----
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
        std::thread t([&]() {
            pc::handle_client(fds[1], "test", db, cache);
        });

        send_line(fds[0], "REGISTER onlyusername");
        std::string r = recv_line(fds[0]);
        CHECK(r == "ERR 1006 Invalid arguments");

        send_line(fds[0], "LOGOUT");  // token eksik
        std::string r2 = recv_line(fds[0]);
        CHECK(r2 == "ERR 1006 Invalid arguments");

        close(fds[0]);
        t.join();
    }

    // ----- Senaryo 5: bilinmeyen komut → 2001 + bağlantı kapanır -----
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
        std::thread t([&]() {
            pc::handle_client(fds[1], "test", db, cache);
        });

        send_line(fds[0], "FOOBAR x y");
        std::string r = recv_line(fds[0]);
        CHECK(r == "ERR 2001 Unknown command");

        // Server bağlantıyı kapatmalı → recv 0 dönmeli
        char c;
        ssize_t n = recv(fds[0], &c, 1, 0);
        CHECK(n == 0);

        close(fds[0]);
        t.join();
    }

    // ----- Senaryo 6: bilinen ama implement edilmemiş komut → 2002, bağlantı açık -----
    {
        int fds[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
        std::thread t([&]() {
            pc::handle_client(fds[1], "test", db, cache);
        });

        // Önce REGISTER, sonra LIST çağır (LIST stub)
        send_line(fds[0], "REGISTER stubuser password1");
        std::string r1 = recv_line(fds[0]);
        CHECK(r1.substr(0, 3) == "OK ");
        std::string token = r1.substr(3);

        send_line(fds[0], "LIST " + token + " newest 0 20");
        std::string r2 = recv_line(fds[0]);
        CHECK(r2 == "ERR 2002 Not implemented yet");

        // Bağlantı hala açık olmalı, başka bir komut işleyebilmeli
        send_line(fds[0], "LOGOUT " + token);
        std::string r3 = recv_line(fds[0]);
        CHECK(r3 == "OK");

        close(fds[0]);
        t.join();
    }
}

// ----------------------------------------------------------------------
int main() {
    test_util();
    test_protocol();
    test_db();
    test_auth();
    test_session();

    std::cout << "\n=========================\n";
    std::cout << "Passed: " << passed << "  Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
