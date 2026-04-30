#pragma once
// Server ve Client için ortak sabitler.
// PROTOCOL.md v1.0 ile uyumlu olmalı.

namespace pc::constants {

// Network
constexpr int    SERVER_PORT      = 8080;
constexpr int    RECV_BUF_SIZE    = 4096;
constexpr int    CHUNK_SIZE       = 64 * 1024;  // 64 KB dosya transferi

// Auth
constexpr int    TOKEN_LENGTH     = 32;          // hex chars
constexpr int    SALT_LENGTH      = 32;          // hex chars
constexpr int    USERNAME_MIN     = 3;
constexpr int    USERNAME_MAX     = 20;
constexpr int    PASSWORD_MIN     = 6;

// Media
constexpr long long MAX_IMAGE_SIZE = 20LL * 1024 * 1024;   // 20 MB
constexpr long long MAX_VIDEO_SIZE = 100LL * 1024 * 1024;  // 100 MB

// LIST sayfalama
constexpr int    LIST_MAX_LIMIT   = 20;

// Hata kodları (PROTOCOL.md bölüm 2)
constexpr int ERR_INVALID_TOKEN       = 1001;
constexpr int ERR_USERNAME_TAKEN      = 1002;
constexpr int ERR_WRONG_CREDENTIALS   = 1003;
constexpr int ERR_NOT_FOUND           = 1004;
constexpr int ERR_UNAUTHORIZED        = 1005;
constexpr int ERR_INVALID_PARAM       = 1006;
constexpr int ERR_FILE_TOO_LARGE      = 1007;
constexpr int ERR_UNSUPPORTED_FORMAT  = 1008;
constexpr int ERR_PRIVATE_MEDIA       = 1009;
constexpr int ERR_BAD_FORMAT          = 2001;
constexpr int ERR_INTERNAL            = 2002;

}
