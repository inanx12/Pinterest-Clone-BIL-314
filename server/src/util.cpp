#include "util.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

namespace pc::util {

// Verilen bir metni SHA-256 algoritmasıyla geri döndürülemez şekilde şifreler (hashler) ve onaltılık (hex) formata çevirir.
// Özellikle kullanıcı şifrelerini (password) veritabanında güvenli bir şekilde saklamak için kullanılır.
std::string sha256_hex(const std::string& input) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    EVP_DigestInit_ex(context, md, nullptr);
    EVP_DigestUpdate(context, input.c_str(), input.length());
    EVP_DigestFinal_ex(context, hash, &lengthOfHash);
    EVP_MD_CTX_free(context);

    std::stringstream ss;
    for (unsigned int i = 0; i < lengthOfHash; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// İstenilen uzunlukta tahmin edilemez ve güvenli rastgele bir değer üretir.
// salt için gerekli.
std::string random_hex(int byte_count) {
    std::vector<unsigned char> buf(byte_count);
    if (RAND_bytes(buf.data(), byte_count) != 1) {
        // OpenSSL random üretemezse normalde fatal'dır; boş döndürüp caller'a bırakıyoruz.
        return "";
    }

    std::stringstream ss;
    for (int i = 0; i < byte_count; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)buf[i];
    }
    return ss.str();
}

// Yeni yüklenen medyalara (resim/video) benzersiz bir ID atamak için 32 karakterlik eşsiz bir UUID üretir.
std::string uuid_hex() {
    // Protokol "32 char UUID hex" istiyor → 16 byte = 32 hex char.
    return random_hex(16);
}

// Şu anki zamanı bilgisayarın genel zaman standardı olan UNIX zaman damgasına (saniye cinsinden) çevirir.
// "created_at" (oluşturulma tarihi) gibi veritabanı kayıtlarında tarih/saat tutmak için kullanılıyor.
long long now_unix() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

}
