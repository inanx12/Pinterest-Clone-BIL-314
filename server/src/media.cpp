#include "media.hpp"
#include "protocol.hpp"
#include "util.hpp"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <sqlite3.h>

namespace pc {

namespace {

// ---- Sabitler ----
constexpr long long IMAGE_MAX = 20LL  * 1024 * 1024;   // 20 MB
constexpr long long VIDEO_MAX = 100LL * 1024 * 1024;   // 100 MB
constexpr size_t    RECV_CHUNK = 64 * 1024;            // 64 KB
constexpr const char* MEDIA_DIR = "data/media";

// TODO (Gün 7): Resim yüklerken thumbnail üretimi BURADA yapılmıyor.
// PREVIEW handler'ı yazılırken ya lazy-generate ederiz ya da bu fonksiyona
// bir blok eklenir. Şimdilik thumb_path = NULL olarak DB'ye giriyor.
// Unutma: stb_image / libjpeg / ImageMagick — birini seçmek gerek.

// Küçük yardımcılar
std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

std::string get_extension(const std::string& filename) {
    auto pos = filename.rfind('.');
    if (pos == std::string::npos || pos == filename.size() - 1) return "";
    return to_lower(filename.substr(pos + 1));
}

bool extension_ok(const std::string& type, const std::string& ext) {
    if (type == "image") return ext == "jpg" || ext == "jpeg" || ext == "png";
    if (type == "video") return ext == "mp4" || ext == "mov"  || ext == "mkv";
    return false;
}

// Filename'e güvenmiyoruz — sadece DB'de gösterim için tutuyoruz, ama
// yine de path traversal denemelerini reddedelim.
bool filename_safe(const std::string& f) {
    if (f.empty() || f.size() > 255) return false;
    if (f.find('/')  != std::string::npos) return false;
    if (f.find('\\') != std::string::npos) return false;
    if (f.find("..") != std::string::npos) return false;
    return true;
}

// session.cpp'dekinin aynısı — partial send'leri toparlar.
bool write_all(int fd, const std::string& data) {
    size_t total = 0;
    while (total < data.size()) {
        ssize_t n = send(fd, data.data() + total, data.size() - total, MSG_NOSIGNAL);
        if (n <= 0) return false;
        total += static_cast<size_t>(n);
    }
    return true;
}

bool insert_media_row(Database& db,
                      const std::string& media_id, int owner_id,
                      const std::string& type, const std::string& visibility,
                      const std::string& filename, long long size,
                      const std::string& file_path) {
    const char* sql =
        "INSERT INTO media (id, owner_id, type, visibility, filename, "
        "size, file_path, thumb_path, like_count, download_count, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, NULL, 0, 0, ?)";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text (stmt, 1, media_id.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_int  (stmt, 2, owner_id);
    sqlite3_bind_text (stmt, 3, type.c_str(),       -1, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 4, visibility.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text (stmt, 5, filename.c_str(),   -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 6, size);
    sqlite3_bind_text (stmt, 7, file_path.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 8, util::now_unix());

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

} // anonim namespace

UploadResult handle_upload(int client_fd, Database& db, int owner_id,
                           const std::string& type,
                           const std::string& visibility,
                           const std::string& filename,
                           long long size,
                           std::string& recv_buf) {
    UploadResult res{};
    res.fatal = false;

    // ===== 1) READY öncesi validation (fatal DEĞİL — bağlantı açık kalır) =====
    if (type != "image" && type != "video") {
        res.error_code = 1008; res.error_msg = "Unsupported type"; return res;
    }
    if (visibility != "public" && visibility != "private") {
        res.error_code = 1006; res.error_msg = "Invalid visibility"; return res;
    }
    if (!filename_safe(filename)) {
        res.error_code = 1006; res.error_msg = "Invalid filename"; return res;
    }
    std::string ext = get_extension(filename);
    if (!extension_ok(type, ext)) {
        res.error_code = 1008; res.error_msg = "Unsupported file format"; return res;
    }
    if (size <= 0) {
        res.error_code = 1006; res.error_msg = "Invalid size"; return res;
    }
    long long max_size = (type == "image") ? IMAGE_MAX : VIDEO_MAX;
    if (size > max_size) {
        res.error_code = 1007; res.error_msg = "File too large"; return res;
    }

    // ===== 2) Hedef dosya yolunu hazırla =====
    std::string media_id  = util::uuid_hex();
    std::string file_path = std::string(MEDIA_DIR) + "/" + media_id + "." + ext;

    std::error_code ec;
    std::filesystem::create_directories(MEDIA_DIR, ec);
    if (ec) {
        res.error_code = 2002; res.error_msg = "Server storage error"; return res;
    }

    std::ofstream out(file_path, std::ios::binary | std::ios::trunc);
    if (!out) {
        res.error_code = 2002; res.error_msg = "Cannot create file"; return res;
    }

    // ===== 3) READY gönder — bu noktadan sonra hatalar FATAL =====
    if (!write_all(client_fd, proto::ready())) {
        out.close();
        std::filesystem::remove(file_path, ec);
        res.error_code = 2002; res.error_msg = "Send failed"; res.fatal = true;
        return res;
    }

    long long remaining = size;

    // ===== 4) Buffer'da kalan byte'ları (varsa) önce dosyaya yaz =====
    // Bu çok önemli: client UPLOAD komutuyla beraber payload'ın bir kısmını
    // aynı TCP paketinde göndermiş olabilir. session'ın read_line'ı zaten
    // bunları okuyup recv_buf'a koymuş.
    if (!recv_buf.empty()) {
        size_t take = std::min<size_t>(recv_buf.size(),
                                       static_cast<size_t>(remaining));
        out.write(recv_buf.data(), take);
        if (!out) {
            out.close();
            std::filesystem::remove(file_path, ec);
            res.error_code = 2002; res.error_msg = "Disk write failed";
            res.fatal = true; return res;
        }
        recv_buf.erase(0, take);
        remaining -= static_cast<long long>(take);
    }

    // ===== 5) Geri kalanı soketten oku, parça parça yaz =====
    char chunk[RECV_CHUNK];
    while (remaining > 0) {
        size_t want = static_cast<size_t>(
            std::min<long long>(RECV_CHUNK, remaining));
        ssize_t n = recv(client_fd, chunk, want, 0);
        if (n <= 0) {
            // Bağlantı koptu / hata → yarım dosyayı temizle
            out.close();
            std::filesystem::remove(file_path, ec);
            res.error_code = 2002; res.error_msg = "Connection lost during upload";
            res.fatal = true; return res;
        }
        out.write(chunk, n);
        if (!out) {
            out.close();
            std::filesystem::remove(file_path, ec);
            res.error_code = 2002; res.error_msg = "Disk write failed";
            res.fatal = true; return res;
        }
        remaining -= n;
    }

    out.close();
    if (!out) {
        std::filesystem::remove(file_path, ec);
        res.error_code = 2002; res.error_msg = "File flush failed";
        res.fatal = true; return res;
    }

    // ===== 6) DB'ye INSERT =====
    if (!insert_media_row(db, media_id, owner_id, type, visibility,
                          filename, size, file_path)) {
        // Dosya diskte ama DB'de yok → orphan olmasın
        std::filesystem::remove(file_path, ec);
        res.error_code = 2002; res.error_msg = "Database error";
        res.fatal = true; return res;
    }

    // ===== 7) Başarılı =====
    res.success  = true;
    res.media_id = media_id;
    return res;
}

ListResult handle_list(Database& db,
                       int requesting_user_id,
                       const std::string& sort,
                       int offset,
                       int limit) {
    ListResult res{};
    res.success = false;

    // ----- 1) Parametre validation -----
    if (offset < 0 || limit <= 0) {
        res.error_code = 1006;
        res.error_msg = "Invalid offset or limit";
        return res;
    }
    if (limit > 20) limit = 20;

    // ----- 2) Sort tipini whitelist'e göre SQL ORDER BY'a çevir -----
    std::string order_by;
    if (sort == "newest") {
        order_by = "m.created_at DESC";
    } else if (sort == "most_liked") {
        order_by = "m.like_count DESC, m.created_at DESC";
    } else if (sort == "most_downloaded") {
        order_by = "m.download_count DESC, m.created_at DESC";
    } else {
        res.error_code = 1006;
        res.error_msg = "Invalid sort";
        return res;
    }

    // ----- 3) SQL sorgusunu hazırla -----
    std::string sql =
        "SELECT m.id, m.type, m.visibility, u.username AS owner, "
        "       m.filename, m.size, m.created_at, "
        "       m.download_count, m.like_count, "
        "       (l.user_id IS NOT NULL) AS liked_by_me "
        "FROM media m "
        "JOIN users u ON u.id = m.owner_id "
        "LEFT JOIN likes l ON l.media_id = m.id AND l.user_id = ? "
        "WHERE m.visibility = 'public' OR m.owner_id = ? "
        "ORDER BY " + order_by + " "
        "LIMIT ? OFFSET ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        res.error_code = 2002;
        res.error_msg = "Database prepare failed";
        return res;
    }

    sqlite3_bind_int(stmt, 1, requesting_user_id);
    sqlite3_bind_int(stmt, 2, requesting_user_id);
    sqlite3_bind_int(stmt, 3, limit);
    sqlite3_bind_int(stmt, 4, offset);

    // ----- 4) Sonuçları JSON array'e doldur -----
    nlohmann::json arr = nlohmann::json::array();

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json item;
        item["id"]             = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item["type"]           = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        item["visibility"]     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        item["owner"]          = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        item["filename"]       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        item["size"]           = sqlite3_column_int64(stmt, 5);
        item["created_at"]     = sqlite3_column_int64(stmt, 6);
        item["download_count"] = sqlite3_column_int64(stmt, 7);
        item["like_count"]     = sqlite3_column_int64(stmt, 8);
        item["liked_by_me"]    = (sqlite3_column_int(stmt, 9) != 0);
        arr.push_back(item);
    }
    sqlite3_finalize(stmt);

    // ----- 5) Sonucu paketle -----
    res.success = true;
    res.count = static_cast<int>(arr.size());
    res.json_payload = arr.dump();
    return res;
}

} // namespace pc
