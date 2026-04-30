#pragma once //başlık dosyasının (header) derleme aşamasında sadece bir kez dahil edilmesini sağlar (Include guard).
#include <sqlite3.h> 
#include <string> 

namespace pc {

class Database {
public:
    // Kurucu metot, veritabanı dosya yolunu parametre olarak alır.
    explicit Database(const std::string& path);
    
    // Yıkıcı metot, bağlantıyı kapatır.
    ~Database();

    // Bellek sızıntılarını ve çökmeleri önlemek için kopyalama ve taşıma işlemlerini siliyoruz.
    // SQLite handle'ı (db_) bir işaretçi (pointer) olduğu için, yanlışlıkla kopyalanması durumunda 
    // birden fazla nesne aynı veritabanı bağlantısını kapatmaya çalışabilir (double free hatası).
    Database(const Database&) = delete; 
    Database& operator=(const Database&) = delete; 
    Database(Database&&) = delete; 
    Database& operator=(Database&&) = delete; 

    // Veritabanı tablolarını ve çalışma modlarını (PRAGMA) ayarlayan başlatma fonksiyonu.
    bool init();

    // Veritabanı sorgularını çalıştırmak için sınıfın dışından doğrudan SQLite bağlantısına (db_) erişimi sağlar.
    sqlite3* handle() { return db_; }

private:
    // SQLite veritabanı bağlantı nesnesini hafızada tutan işaretçi (pointer). 
    // Güvenlik amaçlı varsayılan olarak nullptr (boş/tanımsız) atanır.
    sqlite3* db_ = nullptr;
};

} 
