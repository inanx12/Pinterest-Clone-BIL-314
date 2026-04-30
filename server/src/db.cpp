#include "db.hpp" // Veritabanı sınıfının başlık dosyasını (header) dahil ediyoruz.
#include <iostream> // Hata mesajlarını konsola yazdırmak için iostream kütüphanesini dahil ediyoruz.

namespace pc { // Pinterest clone (pc) isim alanını başlatıyoruz.

// =========================================================================
// Schema embedded — runtime'da dosya okumuyoruz.
// docs/schema.sql ile manuel senkron tutulacak (referans için orada kalsın).
// Schema değişirse iki yerde de güncelle ve commit message'a [SCHEMA] etiketi koy.
// =========================================================================
// Veritabanı tablolarını ve indekslerini oluşturan SQL şeması
static constexpr const char* SCHEMA_SQL = R"SQL(
PRAGMA foreign_keys = ON;

-- Kullanıcı bilgilerini tutan tablo
CREATE TABLE IF NOT EXISTS users (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT    NOT NULL UNIQUE COLLATE NOCASE,
    password_hash   TEXT    NOT NULL,
    salt            TEXT    NOT NULL,
    created_at      INTEGER NOT NULL
);

-- Kullanıcı oturumlarını (token) tutan tablo
CREATE TABLE IF NOT EXISTS sessions (
    token           TEXT    PRIMARY KEY,
    user_id         INTEGER NOT NULL,
    created_at      INTEGER NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Oturum aramalarını hızlandırmak için indeks
CREATE INDEX IF NOT EXISTS idx_sessions_user ON sessions(user_id);

-- Yüklenen medyaları (resim, video) tutan tablo
CREATE TABLE IF NOT EXISTS media (
    id              TEXT    PRIMARY KEY,
    owner_id        INTEGER NOT NULL,
    type            TEXT    NOT NULL CHECK (type IN ('image','video')),
    visibility      TEXT    NOT NULL CHECK (visibility IN ('public','private')),
    filename        TEXT    NOT NULL,
    size            INTEGER NOT NULL,
    file_path       TEXT    NOT NULL,
    thumb_path      TEXT,
    like_count      INTEGER NOT NULL DEFAULT 0,
    download_count  INTEGER NOT NULL DEFAULT 0,
    created_at      INTEGER NOT NULL,
    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Medyaları listelerken sorguları hızlandırmak için oluşturulan indeksler
CREATE INDEX IF NOT EXISTS idx_media_public_newest    ON media(visibility, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_media_public_likes     ON media(visibility, like_count DESC);
CREATE INDEX IF NOT EXISTS idx_media_public_downloads ON media(visibility, download_count DESC);
CREATE INDEX IF NOT EXISTS idx_media_owner            ON media(owner_id, created_at DESC);

-- Medyalara atılan beğenileri tutan tablo
CREATE TABLE IF NOT EXISTS likes (
    user_id         INTEGER NOT NULL,
    media_id        TEXT    NOT NULL,
    created_at      INTEGER NOT NULL,
    PRIMARY KEY (user_id, media_id),
    FOREIGN KEY (user_id)  REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (media_id) REFERENCES media(id) ON DELETE CASCADE
);

-- Medyalara yapılan yorumları tutan tablo
CREATE TABLE IF NOT EXISTS comments (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    media_id        TEXT    NOT NULL,
    author_id       INTEGER NOT NULL,
    text            TEXT    NOT NULL,
    created_at      INTEGER NOT NULL,
    FOREIGN KEY (media_id)  REFERENCES media(id) ON DELETE CASCADE,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
);

-- Belirli bir medyanın yorumlarını hızlıca getirmek için indeks
CREATE INDEX IF NOT EXISTS idx_comments_media ON comments(media_id, created_at DESC);
)SQL";


// Database sınıfının kurucu metodu (Constructor)
// Veritabanı bağlantısını belirtilen dosya yolu (path) üzerinden açar.
Database::Database(const std::string& path) {
    // FULLMUTEX: Aynı bağlantıyı birden fazla thread'den çağırmak güvenli olsun diye thread-safe modda açıyoruz.
    int rc = sqlite3_open_v2(path.c_str(), &db_,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
                             nullptr);
    // Bağlantı başarısız olursa hata mesajı yazdır ve db_ nesnesini temizle
    if (rc != SQLITE_OK) {
        std::cerr << "DB acilamadi: " << sqlite3_errmsg(db_) << std::endl;
        if (db_) { sqlite3_close(db_); }
        db_ = nullptr;
    }
}

// Database sınıfının yıkıcı metodu (Destructor)
// Sınıf ömrünü tamamladığında açık olan veritabanı bağlantısını güvenli bir şekilde kapatır.
Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

// Veritabanı başlatma ve ayarlama metodu
// Tabloların oluşturulması ve PRAGMA ayarlarının yapılması burada gerçekleşir.
bool Database::init() {
    if (!db_) return false; // Eğer db_ bağlantısı yoksa false dön.

    char* errMsg = nullptr;

    // 1) Runtime PRAGMA'lar
    //    - journal_mode=WAL: Yazma işlemleri sırasında okumayı engellememek (concurrent read) için Write-Ahead Logging modunu açar.
    //    - foreign_keys=ON: İlişkisel veri bütünlüğünü (Cascade delete vb.) sağlamak için Foreign Key desteğini aktif eder.
    const char* pragmas =
        "PRAGMA journal_mode = WAL;"
        "PRAGMA foreign_keys = ON;";
    // PRAGMA komutlarını çalıştırıyoruz
    if (sqlite3_exec(db_, pragmas, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "PRAGMA hatasi: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // 2) Schema (embedded — diske bağlılık yok, hardcoded yapmadık)
    // Yukarıda tanımlanan SQL şemasını çalıştırarak tabloları ve indeksleri oluşturur. 
    // (Zaten varsa IF NOT EXISTS sayesinde mevcut tablolara dokunmaz)
    if (sqlite3_exec(db_, SCHEMA_SQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Schema olusturma hatasi: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true; // Her şey başarılıysa true dön.
}

}
