#pragma once
#include <string>
#include <optional>
#include <sqlite3.h>

namespace pc {
class Database {
public:
    Database(const std::string& path);
    ~Database();
    
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool init();           // schema.sql çalıştır, PRAGMA'ları set et
    sqlite3* handle() { return db_; }
    
private:
    sqlite3* db_ = nullptr;
};
}
