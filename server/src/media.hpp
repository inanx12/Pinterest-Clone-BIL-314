#pragma once
#include "db.hpp"
#include <string>

namespace pc {

struct UploadResult {
    bool success;          // başarılı mı
    std::string media_id;  // başarılıysa yeni medyanın id'si
    int error_code;        // hata varsa kod (protokol §2)
    std::string error_msg; // hata mesajı
    bool fatal;            // true → session bağlantıyı kapatmalı
                           //         (akış bozuldu, tekrar senkron olamayız)
};

// UPLOAD komutunu işler.
// Çağırmadan önce session.cpp:
//   - token'ı doğrulamalı (owner_id buradan geliyor)
//   - argümanları parse etmiş olmalı (5 arg: token/type/vis/filename/size)
//
// recv_buf: session'ın kalan-byte buffer'ı. UPLOAD'a girerken içinde zaten
//           binary payload başlangıcı olabilir. Önce o tüketilir, sonra
//           soketten okumaya geçilir.
UploadResult handle_upload(int client_fd,
                           Database& db,
                           int owner_id,
                           const std::string& type,
                           const std::string& visibility,
                           const std::string& filename,
                           long long size,
                           std::string& recv_buf);



struct ListResult {
    bool success;
    std::string json_payload;  // başarılıysa: "[{...},{...}]" formatında
    int count;                 // kaç kayıt döndü
    int error_code;
    std::string error_msg;
};

// LIST komutunu işler.
// requesting_user_id: o anki kullanıcının id'si (liked_by_me ve private filtreleme için)
// sort: "newest" | "most_liked" | "most_downloaded"
// offset, limit: sayfalama (limit max 20)
ListResult handle_list(Database& db,
                       int requesting_user_id,
                       const std::string& sort,
                       int offset,
                       int limit);



struct DownloadResult {
    bool success;
    int error_code;
    std::string error_msg;
    bool fatal;             // true → session bağlantıyı kapatmalı
                            //   (binary akışı yarım kaldı, senkronu kaybettik)
};

// DOWNLOAD komutunu işler.
// Akış: validate → DB lookup → private check → diskten oku → header gönder
//       → chunked send → download_count++.
//
// Header (OK <type> <size>\n) gönderilmeden önceki hatalar non-fatal.
// Header gönderildikten sonraki hatalar fatal (client binary bekliyor).
DownloadResult handle_download(int client_fd,
                               Database& db,
                               int requesting_user_id,
                               const std::string& media_id);




struct PreviewResult {
    bool success;
    int error_code;
    std::string error_msg;
    bool fatal;             // header gönderildikten sonra hata → bağlantı kapat
};

// PREVIEW komutu — resim için 256x256 (aspect-preserving) JPEG thumbnail döndürür.
// Lazy: data/thumbs/<media_id>.jpg dosyası yoksa anında üretip diske cache'ler.
// Video için ERR 1006 (preview yok). Private medya sadece sahibine.
PreviewResult handle_preview(int client_fd,
                             Database& db,
                             int requesting_user_id,
                             const std::string& media_id);




// =========================================================================
// LIKE / UNLIKE
// =========================================================================
// Aynı struct hem LIKE hem UNLIKE için kullanılır.
// Protokol §4.9: cevap "OK <new_like_count>\n"
//
// Idempotent: aynı kullanıcı tekrar LIKE atarsa count artmaz, yine OK döner.
// Aynı şekilde UNLIKE: zaten beğenmemişse de OK + mevcut count döner.
// Private medya beğenilemez → 1004 (varlığını leak etmemek için).
struct LikeResult {
    bool success;
    long long new_like_count;  // başarılıysa: güncel like sayısı
    int error_code;
    std::string error_msg;
};

LikeResult handle_like(Database& db,
                       int requesting_user_id,
                       const std::string& media_id);

LikeResult handle_unlike(Database& db,
                         int requesting_user_id,
                         const std::string& media_id);

// =========================================================================
// DELETE
// =========================================================================
// Sadece sahibi silebilir. Hard delete:
//   - DB'den media row silinir → likes ve comments cascade ile gider
//   - Disk: data/media/<id>.<ext> ve data/thumbs/<id>.jpg silinir
// Sıra: DB önce, disk sonra (orphan dosya tolere edilir, orphan kayıt edilmez).
struct DeleteResult {
    bool success;
    int error_code;
    std::string error_msg;
};

DeleteResult handle_delete(Database& db,
                           int requesting_user_id,
                           const std::string& media_id);

// =========================================================================
// USER_MEDIA
// =========================================================================
// LIST'in profil versiyonu. Filtre farkı:
//   - target_username'in medyaları
//   - requesting_user == target ise public+private, değilse sadece public
//   - target_username yoksa → ERR 1004
//
// Cevap formatı LIST ile birebir aynı, ListResult yeniden kullanılıyor.
ListResult handle_user_media(Database& db,
                             int requesting_user_id,
                             const std::string& target_username,
                             int offset,
                             int limit);




}
