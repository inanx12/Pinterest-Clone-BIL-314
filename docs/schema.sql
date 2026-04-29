-- Pinterest-Clone-BIL-314 — Database Schema v1.0
-- SQLite3
-- Protokol referansı: docs/PROTOCOL.md v1.0
--
-- Notlar:
--  - WAL mode: ayrı PRAGMA komutuyla server başlangıcında set edilecek
--    (CREATE TABLE'larla birlikte saklanmıyor, runtime ayarı)
--  - Foreign key constraint'leri çalışsın diye her bağlantıda
--    PRAGMA foreign_keys = ON; gerekiyor.
--  - Tüm timestamp'ler unix epoch (saniye), INTEGER olarak.

PRAGMA foreign_keys = ON;

-- =========================================================================
-- users
-- =========================================================================
-- password_hash: SHA256(password + salt), hex string (64 char)
-- salt:          per-user random 16 byte, hex string (32 char)
CREATE TABLE IF NOT EXISTS users (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    username        TEXT    NOT NULL UNIQUE COLLATE NOCASE,
    password_hash   TEXT    NOT NULL,
    salt            TEXT    NOT NULL,
    created_at      INTEGER NOT NULL
);

-- Username lookup login/register'da çok kullanılacak.
-- UNIQUE constraint zaten index oluşturur ama COLLATE NOCASE açıkça duruyor.

-- =========================================================================
-- sessions
-- =========================================================================
-- Token: 32 char hex random. Multi-device: bir user'ın birden fazla satırı olabilir.
-- LOGOUT veya CHANGE_PASSWORD'da ilgili satır(lar) silinir.
CREATE TABLE IF NOT EXISTS sessions (
    token           TEXT    PRIMARY KEY,
    user_id         INTEGER NOT NULL,
    created_at      INTEGER NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_sessions_user ON sessions(user_id);
-- CHANGE_PASSWORD'da user'ın tüm tokenlarını silerken lazım.

-- =========================================================================
-- media
-- =========================================================================
-- file_path: diskte fiziksel yol (örn. data/media/<media_id>.<ext>)
-- thumb_path: sadece type='image' için doluyor
-- like_count, download_count: denormalize. Mutex altında atomik update.
CREATE TABLE IF NOT EXISTS media (
    id              TEXT    PRIMARY KEY,        -- 32 char UUID hex
    owner_id        INTEGER NOT NULL,
    type            TEXT    NOT NULL CHECK (type IN ('image','video')),
    visibility      TEXT    NOT NULL CHECK (visibility IN ('public','private')),
    filename        TEXT    NOT NULL,           -- orijinal dosya adı (gösterim için)
    size            INTEGER NOT NULL,           -- byte
    file_path       TEXT    NOT NULL,           -- disk yolu
    thumb_path      TEXT,                       -- sadece resim için
    like_count      INTEGER NOT NULL DEFAULT 0,
    download_count  INTEGER NOT NULL DEFAULT 0,
    created_at      INTEGER NOT NULL,
    FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE
);

-- LIST sorguları için indexler (sort: newest, most_downloaded, most_liked)
-- Sadece public medyalar feed'de görünür, ama herkes kendi private'larını da
-- görüyor (USER_MEDIA self), o yüzden visibility'yi ayrı index'e koymadık.
-- WHERE visibility='public' ORDER BY created_at DESC sorgusu için:
CREATE INDEX IF NOT EXISTS idx_media_public_newest    ON media(visibility, created_at DESC);
CREATE INDEX IF NOT EXISTS idx_media_public_likes     ON media(visibility, like_count DESC);
CREATE INDEX IF NOT EXISTS idx_media_public_downloads ON media(visibility, download_count DESC);

-- USER_MEDIA için: belirli owner'ın medyalarını created_at'e göre sırala
CREATE INDEX IF NOT EXISTS idx_media_owner ON media(owner_id, created_at DESC);

-- =========================================================================
-- likes
-- =========================================================================
-- Composite PK: aynı user aynı media'yı tekrar beğenemesin (idempotent).
CREATE TABLE IF NOT EXISTS likes (
    user_id         INTEGER NOT NULL,
    media_id        TEXT    NOT NULL,
    created_at      INTEGER NOT NULL,
    PRIMARY KEY (user_id, media_id),
    FOREIGN KEY (user_id)  REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (media_id) REFERENCES media(id) ON DELETE CASCADE
);

-- LIST cevabındaki liked_by_me için (user_id, media_id) PK'sı yetiyor.
-- Ekstra: bir medyanın tüm beğenenlerini listelemek istersek (şimdilik yok)
-- media_id üzerine index lazım olabilir. Şimdilik atlıyoruz.

-- =========================================================================
-- comments  (STRETCH GOAL — protokol bölüm 5)
-- =========================================================================
-- Şimdiden tabloyu yaratıyoruz; vakit kalmazsa boş kalır, schema değişmez.
CREATE TABLE IF NOT EXISTS comments (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    media_id        TEXT    NOT NULL,
    author_id       INTEGER NOT NULL,
    text            TEXT    NOT NULL,           -- max 500 char (server-side check)
    created_at      INTEGER NOT NULL,
    FOREIGN KEY (media_id)  REFERENCES media(id) ON DELETE CASCADE,
    FOREIGN KEY (author_id) REFERENCES users(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_comments_media ON comments(media_id, created_at DESC);
