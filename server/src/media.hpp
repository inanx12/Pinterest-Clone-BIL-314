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

}
