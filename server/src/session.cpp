#include "session.hpp"
#include "auth.hpp"
#include "protocol.hpp"
#include "util.hpp"
#include "media.hpp"

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace pc {

namespace {

constexpr size_t RECV_BUF = 4096;
constexpr size_t MAX_LINE = 8192;  // bozuk client DOS etmesin

// =========================================================================
// Bir komut satırı oku (\n'e kadar). Persistent buffer state tutar:
// recv() bir çağrıda iki komut + yarım komut getirebilir, fazlayı saklarız.
// Dönüş: true = satır hazır, false = bağlantı kapandı / hata / max line aşıldı.
// =========================================================================
bool read_line(int fd, std::string& buf, std::string& out_line) {
    while (true) {
        // Önce mevcut buf'ta \n var mı bak
        size_t nl = buf.find('\n');
        if (nl != std::string::npos) {
            out_line = buf.substr(0, nl);
            buf.erase(0, nl + 1);
            // Trailing \r temizle (CRLF)
            if (!out_line.empty() && out_line.back() == '\r') out_line.pop_back();
            return true;
        }
        if (buf.size() > MAX_LINE) return false;

        char tmp[RECV_BUF];
        ssize_t n = recv(fd, tmp, sizeof(tmp), 0);
        if (n <= 0) return false;  // 0 = peer closed, <0 = error
        buf.append(tmp, n);
    }
}

// send() partial write yapabilir, hepsi gidene kadar döngü.
bool write_all(int fd, const std::string& data) {
    size_t total = 0;
    while (total < data.size()) {
        ssize_t n = send(fd, data.data() + total, data.size() - total, MSG_NOSIGNAL);
        if (n <= 0) return false;
        total += static_cast<size_t>(n);
    }
    return true;
}

void log_cmd(const std::string& ip, const std::string& username,
             const std::string& cmd_name, const std::string& result) {
    // Argümanları (şifre, token!) ASLA log'a basma. Sadece komut adı + sonuç.
    std::cout << "[" << util::now_unix() << "] [" << ip << "] ["
              << (username.empty() ? "-" : username) << "] "
              << (cmd_name.empty() ? "<empty>" : cmd_name)
              << " -> " << result << "\n";
}

} // anon namespace

// =========================================================================
void handle_client(int client_fd, const std::string& client_ip,
                   Database& db, SessionCache& cache) {
    std::string buf;
    std::string current_user;  // log için, LOGIN/REGISTER sonrası set edilir

    while (true) {
        std::string line;
        if (!read_line(client_fd, buf, line)) break;

        auto cmd = proto::parse_command(line);

        // ----- REGISTER -----
        if (cmd.name == "REGISTER") {
            if (cmd.args.size() != 2) {
                write_all(client_fd, proto::err(1006, "Invalid arguments"));
                log_cmd(client_ip, current_user, cmd.name, "ERR 1006");
                continue;
            }
            auto r = register_user(db, cache, cmd.args[0], cmd.args[1]);
            if (r.success) {
                current_user = cmd.args[0];
                write_all(client_fd, proto::ok(r.token));
                log_cmd(client_ip, current_user, cmd.name, "OK");
            } else {
                write_all(client_fd, proto::err(r.error_code, r.error));
                log_cmd(client_ip, current_user, cmd.name,
                        "ERR " + std::to_string(r.error_code));
            }
            continue;
        }

        // ----- LOGIN -----
        if (cmd.name == "LOGIN") {
            if (cmd.args.size() != 2) {
                write_all(client_fd, proto::err(1006, "Invalid arguments"));
                log_cmd(client_ip, current_user, cmd.name, "ERR 1006");
                continue;
            }
            auto r = login_user(db, cache, cmd.args[0], cmd.args[1]);
            if (r.success) {
                current_user = cmd.args[0];
                write_all(client_fd, proto::ok(r.token));
                log_cmd(client_ip, current_user, cmd.name, "OK");
            } else {
                write_all(client_fd, proto::err(r.error_code, r.error));
                log_cmd(client_ip, current_user, cmd.name,
                        "ERR " + std::to_string(r.error_code));
            }
            continue;
        }

        // ----- LOGOUT -----
        if (cmd.name == "LOGOUT") {
            if (cmd.args.size() != 1) {
                write_all(client_fd, proto::err(1006, "Invalid arguments"));
                log_cmd(client_ip, current_user, cmd.name, "ERR 1006");
                continue;
            }
            logout(db, cache, cmd.args[0]);
            // İdempotent: var olmayan token logout'u bile OK (protokol §4.3'te ERR yok).
            write_all(client_fd, proto::ok());
            log_cmd(client_ip, current_user, cmd.name, "OK");
            current_user.clear();
            continue;
        }

        // ----- UPLOAD -----
	if (cmd.name == "UPLOAD") {
    	    if (cmd.args.size() != 5) {
                write_all(client_fd, proto::err(1006, "Invalid arguments"));
                log_cmd(client_ip, current_user, cmd.name, "ERR 1006");
                continue;
            }
            auto user = validate_token(db, cache, cmd.args[0]);
            if (!user) {
                write_all(client_fd, proto::err(1001, "Invalid or expired token"));
        	log_cmd(client_ip, current_user, cmd.name, "ERR 1001");
        	continue;
            }
    	    long long size = 0;
    	    try {
                size = std::stoll(cmd.args[4]);
            } catch (...) {
        	write_all(client_fd, proto::err(1006, "Invalid size"));
        	log_cmd(client_ip, user->username, cmd.name, "ERR 1006");
        	continue;
    	    }
    	    auto r = handle_upload(client_fd, db, user->id,
                                   cmd.args[1], cmd.args[2], cmd.args[3],
                                   size, buf);
    	    if (r.success) {
                write_all(client_fd, proto::ok(r.media_id));
        	log_cmd(client_ip, user->username, cmd.name, "OK");
    	    } else {
        	write_all(client_fd, proto::err(r.error_code, r.error_msg));
        	log_cmd(client_ip, user->username, cmd.name,
                	"ERR " + std::to_string(r.error_code) +
                	(r.fatal ? " (close)" : ""));
        	if (r.fatal) break;
    	    }
    	    continue;
	}

	// ----- LIST -----
        if (cmd.name == "LIST") {
            if (cmd.args.size() != 4) {
                write_all(client_fd, proto::err(1006, "Invalid arguments"));
                log_cmd(client_ip, current_user, cmd.name, "ERR 1006");
                continue;
            }
            auto user = validate_token(db, cache, cmd.args[0]);
            if (!user) {
                write_all(client_fd, proto::err(1001, "Invalid or expired token"));
                log_cmd(client_ip, current_user, cmd.name, "ERR 1001");
                continue;
            }

            int offset = 0, limit = 0;
            try {
                offset = std::stoi(cmd.args[2]);
                limit  = std::stoi(cmd.args[3]);
            } catch (...) {
                write_all(client_fd, proto::err(1006, "Invalid offset or limit"));
                log_cmd(client_ip, user->username, cmd.name, "ERR 1006");
                continue;
            }

            auto r = handle_list(db, user->id, cmd.args[1], offset, limit);
            if (r.success) {
                // Cevap formatı: "OK <count>\n<json>\n"
                std::string payload = std::to_string(r.count) + "\n" + r.json_payload;
                write_all(client_fd, proto::ok(payload));
                log_cmd(client_ip, user->username, cmd.name,
                        "OK (" + std::to_string(r.count) + ")");
            } else {
                write_all(client_fd, proto::err(r.error_code, r.error_msg));
                log_cmd(client_ip, user->username, cmd.name,
                        "ERR " + std::to_string(r.error_code));
            }
            continue;
        }

        // ----- Bozuk format / boş komut: bağlantıyı kapat (protokol §7) -----
        if (cmd.name.empty()) {
            write_all(client_fd, proto::err(2001, "Malformed command"));
            log_cmd(client_ip, current_user, "<empty>", "ERR 2001 (close)");
            break;
        }

        // ----- Bilinmeyen komut: yine 2001 + close (parser senkronu) -----
        // (Protokol §7: "Bozuk format ... bağlantı kapatılır")
        // Şu anda implement edilmemiş bilinen komutlar için 2002 dönüp bağlantıyı
        // KAPATMIYORUZ — ileride ekleneceğinde aynı oturum çalışmaya devam etsin.
        static const char* known_but_not_yet[] = {
            "DOWNLOAD", "PREVIEW", "DELETE",
            "LIKE", "UNLIKE", "USER_MEDIA", "CHANGE_PASSWORD",
            "ADD_COMMENT", "LIST_COMMENTS", "DELETE_COMMENT"
        };
        bool is_known = false;
        for (const char* k : known_but_not_yet) {
            if (cmd.name == k) { is_known = true; break; }
        }
        if (is_known) {
            write_all(client_fd, proto::err(2002, "Not implemented yet"));
            log_cmd(client_ip, current_user, cmd.name, "ERR 2002 (stub)");
            continue;
        }

        // Gerçekten bilinmeyen komut → kapat
        write_all(client_fd, proto::err(2001, "Unknown command"));
        log_cmd(client_ip, current_user, cmd.name, "ERR 2001 (close)");
        break;
    }

    close(client_fd);
}

}
