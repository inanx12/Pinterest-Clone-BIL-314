# SERVER Tarafı Seçtiysen — Yol Haritası

Selam! Server tarafını seçtin. Bu rehber sana **ne yapacağını, hangi sırayla, AI'a ne sorman gerektiğini** anlatıyor.

---

## Server Ne İşe Yarıyor?

Sen **arka planda çalışan** kısmı yapıyorsun. Kullanıcı arayüzü yok, terminal'de çalışan bir program. İşi:

- TCP üstünden istemcileri kabul etmek (port 8080)
- Gelen komutları parse etmek (REGISTER, LOGIN, UPLOAD vs.)
- Veritabanına yazıp okumak (SQLite)
- Dosyaları diske kaydedip okumak (resim/video)
- Şifreyi hash'lemek, token üretmek
- Cevap döndürmek

Pinterest sunucusu gibi düşün. Telefondaki uygulama bağlanır, sen "kim bu, ne istiyor" diye bakıp cevap verirsin.

---

## Çalışma Ortamın

- **OS:** Linux (WSL Ubuntu üstünde)
- **Dil:** C++17
- **Build:** CMake
- **DB:** SQLite3
- **Network:** POSIX socket (Beej's Guide stili)
- **Threading:** std::thread (her client'a ayrı thread)
- **Hash:** OpenSSL (SHA256)

Hepsi WSL'de zaten kurulu, bir şey eklemen gerekmiyor (eğer kurulum rehberini bitirdiysen).

---

## Klasör Yapısı

```
server/
├── CMakeLists.txt          ← build config
├── src/
│   ├── main.cpp            ← program girişi (server'ı başlat)
│   ├── server.hpp/cpp      ← TCP listener + accept loop
│   ├── session.hpp/cpp     ← her client için thread fonksiyonu
│   ├── protocol.hpp/cpp    ← komut parser (REGISTER, LOGIN vs.)
│   ├── db.hpp/cpp          ← SQLite wrapper
│   ├── auth.hpp/cpp        ← REGISTER, LOGIN, token kontrolü
│   ├── media.hpp/cpp       ← UPLOAD, DOWNLOAD, LIST, DELETE
│   └── util.hpp/cpp        ← SHA256, UUID, timestamp helpers
└── build/                  ← cmake derler buraya (gitignore'da)
```

---

## Yol Haritası — Hangi Sırayla Yapacaksın

### Aşama 1: Yardımcı modüller (Gün 3-4)

Önce **temel yapı taşları**, sonra üstüne komutları ekleyeceksin.

#### İş 1.1: util.hpp/cpp (1 saat)

**Ne yapacak:**
- `sha256_hex(string)` → şifreyi hash'le
- `random_hex(int byte_count)` → token ve salt için rastgele hex
- `uuid_hex()` → media_id için 32 karakter UUID
- `now_unix()` → şu anki timestamp (saniye)

**AI'a sor:**
```
util.cpp dosyasını yazmak istiyorum. util.hpp'de şu fonksiyonlar tanımlı:
[util.hpp içeriğini yapıştır]

OpenSSL'in SHA256 fonksiyonunu kullanarak (libcrypto), basit ve thread-safe
bir implementasyon yaz. Her fonksiyonun üstüne kısa açıklama koy.
```

**Test et:**
```cpp
// main.cpp'de geçici test:
std::cout << pc::util::sha256_hex("password123") << "\n";
std::cout << pc::util::random_hex(16) << "\n";
std::cout << pc::util::now_unix() << "\n";
```

#### İş 1.2: db.hpp/cpp (1.5 saat)

**Ne yapacak:**
- SQLite veritabanı dosyasını aç (`pinterest.db`)
- `schema.sql` dosyasını çalıştır (tablolar yoksa oluşturur)
- `PRAGMA foreign_keys = ON` ve `PRAGMA journal_mode = WAL` ayarla
- Diğer modüllere `sqlite3*` handle versin

**AI'a sor:**
```
db.cpp dosyasını yazmak istiyorum. db.hpp'de Database sınıfı tanımlı:
[db.hpp içeriğini yapıştır]

İşte schema.sql:
[schema.sql içeriğini yapıştır]

init() fonksiyonu:
1. SQLite dosyasını açsın
2. PRAGMA foreign_keys = ON çalıştırsın
3. PRAGMA journal_mode = WAL çalıştırsın
4. schema.sql dosyasını okuyup çalıştırsın (CREATE TABLE IF NOT EXISTS)

Hata kontrolü ile birlikte yaz.
```

**Test et:**
```cpp
// main.cpp'de:
pc::Database db("pinterest.db");
if (!db.init()) {
    std::cerr << "DB init failed\n";
    return 1;
}
std::cout << "DB ready\n";
```

`pinterest.db` dosyası oluşmalı, `sqlite3 pinterest.db` ile açıp `.tables` dersen tabloları görmelisin.

#### İş 1.3: protocol.hpp/cpp (1 saat)

**Ne yapacak:**
- Gelen komut satırını parse et (örn `REGISTER inan password123`)
- `Command` struct döndür (name + args)
- Cevap formatlayıcılar: `ok()`, `ok(payload)`, `err(code, msg)`

**AI'a sor:**
```
protocol.cpp dosyasını yazmak istiyorum. protocol.hpp'de tanımlı:
[protocol.hpp içeriğini yapıştır]

Komut formatı: "<COMMAND> <arg1> <arg2> ...\n"
Argümanlar boşlukla ayrılır, boşluk içeren argüman olamaz (v1'de).
ADD_COMMENT komutu istisna - son argüman satır sonuna kadar text olabilir
ama bunu sonra düşüneceğiz, şimdi standart parser yaz.

Cevap formatları PROTOCOL.md'ye göre:
- "OK\n"
- "OK <payload>\n"
- "ERR <code> <message>\n"
```

**Test et:**
```cpp
auto cmd = pc::proto::parse_command("REGISTER inan password123\n");
std::cout << "name: " << cmd.name << "\n";
for (auto& a : cmd.args) std::cout << "  arg: " << a << "\n";
```

### Aşama 2: Auth (Gün 4-5)

#### İş 2.1: auth.cpp - REGISTER (1.5 saat)

**Ne yapacak:**
- Username validation (3-20 char, alphanumeric + _)
- Password validation (min 6 char)
- Username zaten var mı kontrol
- Salt üret, hash hesapla
- DB'ye INSERT
- Token üret, sessions tablosuna INSERT
- Token döndür

**AI'a sor:**
```
auth.cpp içinde register_user fonksiyonunu yazmak istiyorum.
İmza: RegisterResult register_user(Database& db, const std::string& username,
                                    const std::string& password);

PROTOCOL.md 4.1'e göre davransın. Hata kodları:
- 1002 username zaten var
- 1006 geçersiz parametre

SQLite prepared statement kullan (SQL injection'a karşı).
util.hpp'deki sha256_hex ve random_hex'i kullan.

Şu anda elimde olanlar:
[auth.hpp]
[db.hpp]
[util.hpp]
```

**Test et:**
Önce manuel test (ileride session.cpp'ye taşıyacaksın):
```cpp
auto r = pc::register_user(db, "inan", "password123");
if (r.success) std::cout << "Token: " << r.token << "\n";
else std::cout << "Hata: " << r.error_code << " " << r.error << "\n";
```

#### İş 2.2: auth.cpp - LOGIN (45 dk)

REGISTER'a benzer ama:
- Username/password al
- DB'den salt + hash çek
- Hash karşılaştır
- Doğruysa yeni token üret, sessions'a INSERT, dön

#### İş 2.3: auth.cpp - validate_token (30 dk)

- Token alıp sessions tablosunda var mı bak
- Varsa user_id ve username dön (User struct)
- Yoksa nullopt dön

Bu fonksiyon **her komutta** çağrılacak (REGISTER ve LOGIN hariç). Hızlı olmalı, primary key lookup zaten hızlı.

#### İş 2.4: session.cpp - main dispatch loop (2 saat)

**Ne yapacak:**
- Client'tan satır oku (recv ile, `\n` görene kadar)
- protocol::parse_command ile parse et
- Komutuna göre uygun handler'a yönlendir:
  - REGISTER → auth::register_user → cevap dön
  - LOGIN → auth::login_user → cevap dön
  - LOGOUT → auth::logout → cevap dön
  - Diğerleri → token validate → media::xxx → cevap dön

**AI'a sor:**
```
session.cpp dosyasında handle_client fonksiyonunu yazmak istiyorum.
Bu fonksiyon her client için ayrı thread'de çalışacak.

Akış:
1. Sonsuz döngü: client'tan satır oku (recv loop, \n bekleyen)
2. Boş satır veya bağlantı kopması → loop'tan çık
3. protocol::parse_command ile parse et
4. Komuta göre dispatch:
   - REGISTER, LOGIN: auth modülüne git
   - LOGOUT: auth::logout
   - Diğerleri: önce auth::validate_token, geçersizse ERR 1001
   - UPLOAD, LIST, DOWNLOAD, DELETE, LIKE, UNLIKE, USER_MEDIA → media modülü
5. Sonuç string'ini send ile gönder

İşte session.hpp:
[session.hpp]

Diğer modülleri include edip kullan.
```

#### Milestone Test:

Bu aşama bitince ilk gerçek test:

Terminal 1:
```bash
./server
```

Terminal 2:
```bash
nc localhost 8080
REGISTER inan testpass
# Cevap: OK <token>
LOGIN inan testpass
# Cevap: OK <başka_token>
LOGIN inan yanlissifre
# Cevap: ERR 1003 Wrong username or password
```

✅ Bu çalışıyorsa **temel auth bitmiş** demektir, çok önemli milestone.

### Aşama 3: Media (Gün 5-7)

#### İş 3.1: UPLOAD handler (3-4 saat — EN ZOR KISIM)

**Ne yapacak:**
1. Token validate
2. UPLOAD komutu: type, visibility, filename, size oku
3. Boyut/format kontrolü, hata varsa ERR
4. Yoksa "READY\n" gönder
5. Client'tan tam `size` byte oku (CHUNK CHUNK!)
6. Diske yaz
7. UUID üret, media tablosuna INSERT
8. Resimse thumbnail oluştur (stb_image_resize veya basitçe atla)
9. "OK <media_id>\n" dön

**KRITIK:**
TCP'de `recv()` istediğin kadar byte vermez, **döngü zorunlu**:

```cpp
size_t total_read = 0;
char buf[64*1024];  // 64KB chunk
while (total_read < expected_size) {
    int n = recv(fd, buf, std::min(sizeof(buf), expected_size - total_read), 0);
    if (n <= 0) break; // hata veya bağlantı koptu
    file.write(buf, n);
    total_read += n;
}
```

**AI'a sor:**
```
media.cpp içinde handle_upload fonksiyonunu yazmak istiyorum.
PROTOCOL.md 4.4 UPLOAD davranışı:

1. UPLOAD <token> <type> <visibility> <filename> <size>\n geliyor
2. Validation: 
   - type image|video
   - visibility public|private
   - size <= 20MB (image) veya 100MB (video)
   - Uzantı kontrolü: image → jpg/jpeg/png, video → mp4/mov/mkv
3. Hata varsa ERR dön
4. OK ise READY\n gönder
5. Client'tan tam size byte oku (chunked!)
6. data/<uuid>.<ext> olarak diske yaz
7. media tablosuna INSERT (id=uuid, owner_id, type, visibility, filename, size, file_path)
8. OK <uuid>\n dön

Thread safety: SQLite'a aynı anda iki thread INSERT atarsa olabilir mi?
WAL mode'da READ paralel ama WRITE serialize. Şimdilik mutex koymadan dene.

Chunk read için recv döngüsü kritik. 64KB chunk yeterli.
```

#### İş 3.2: LIST handler (1.5 saat)

**Ne yapacak:**
1. Token validate
2. sort, offset, limit parse
3. SQL query: 
   ```sql
   SELECT m.id, m.type, m.visibility, u.username AS owner, ...
   FROM media m JOIN users u ON m.owner_id = u.id
   WHERE m.visibility = 'public' OR m.owner_id = ?
   ORDER BY [created_at|like_count|download_count] DESC
   LIMIT ? OFFSET ?
   ```
4. liked_by_me için subquery veya LEFT JOIN
5. Sonuçları JSON'a çevir
6. `OK <count>\n<json>\n` dön

**AI'a sor:**
```
media.cpp içinde handle_list fonksiyonunu yazmak istiyorum.
[detayları yapıştır]

JSON oluşturmak için kütüphane kullanmak istemiyorum (extra dep), elle string concat yapacağım. 
Veya nlohmann/json'u single-header olarak ekleyebilirim, sen ne dersin?
```

#### İş 3.3: DOWNLOAD handler (1.5 saat)

**Ne yapacak:**
1. Token validate
2. media_id ile DB'den dosya bilgisi çek
3. Eğer private ve owner değilse → ERR 1009
4. `OK <type> <size>\n` gönder
5. Dosyayı diskten oku, **chunk chunk** gönder
6. download_count++ (UPDATE)

**Kritik:** UPLOAD gibi `send` döngüsü:
```cpp
while (total_sent < file_size) {
    int n = send(fd, buf + total_sent, file_size - total_sent, 0);
    if (n <= 0) break;
    total_sent += n;
}
```

#### İş 3.4: DELETE, LIKE, UNLIKE, USER_MEDIA (toplam 2 saat)

Hepsi DOWNLOAD'a göre kolay, küçük SQL query'leri.

### Aşama 4: Polish ve Edge Cases (Gün 8-9)

#### İş 4.1: PREVIEW handler (resim için thumbnail)

**Karar:** Şimdilik **server'da thumbnail oluşturmayı atla**, client tarafında küçültsün. Server'a tam dosya kaydedilir, client küçültülmüşünü gösterir. Bu büyük basitleştirme.

Eğer hoca direkt sorarsa "thumbnail server'da olmadığı için preview = küçük resim, client tarafında küçültülüyor" diye savun.

#### İş 4.2: CHANGE_PASSWORD

Login'e benzer ama old/new password alır, eski hash kontrol, yeni hash yaz, **tüm token'ları sil** (sessions WHERE user_id = ?), yeni token üret.

#### İş 4.3: Edge cases

- Bozuk komut → ERR 2001 + connection close
- Bağlantı koparsa upload sırasında → yarım dosyayı sil
- Çok uzun komut satırı → reddet
- SQL injection → prepared statement her yerde olmalı (mutlaka kontrol et!)
- Çok büyük dosya → max boyutu erkenden kontrol

### Aşama 5: VPS Deploy (Gün 8)

Bunu İnan'la birlikte yapın:
1. Hetzner CX22 hesap, sunucu aç (Ubuntu 22.04)
2. SSH ile gir, Git clone et, derle
3. systemd service yap → 7/24 çalışsın
4. Public IP'yi client'a ver

---

## Test Stratejisi

Her özellik için:
1. **Unit-ish test:** Manuel olarak `nc` ile komut gönder
2. **Pozitif test:** Beklediğin gibi çalışıyor mu
3. **Negatif test:** Yanlış input verince doğru hata mı
4. **Edge case:** Boş, çok uzun, geçersiz karakter

Örnek test seti:
```bash
nc localhost 8080
REGISTER inan pass123        # OK
REGISTER inan başka          # ERR 1002 (zaten var)
LOGIN inan pass123           # OK
LOGIN inan yanlış            # ERR 1003
LIST <token> newest 0 20     # OK 0 [] (boş)
DELETE <token> abc           # ERR 1004 (yok)
INVALIDCMD                   # ERR 2001 (bağlantı kapanır)
```

---

## Sıkça Karşılaşacağın Sorunlar

### "SQLite3 not found" derleme hatası
```bash
sudo apt install libsqlite3-dev
```

### "OpenSSL not found"
```bash
sudo apt install libssl-dev
```

### "Address already in use"
Önceki server hâlâ çalışıyor (8080 portu meşgul). Bul ve öldür:
```bash
lsof -i :8080
kill -9 <pid>
```

Ya da kodda `SO_REUSEADDR` zaten var, normal kapatınca tekrar açılır.

### Recv 0 byte dönüyor
Bağlantı koptu demektir, döngüden çık.

### SQLite "database is locked"
İki thread aynı anda WRITE yapıyor. WAL mode bunu çoğunlukla çözer. Hala olursa kritik bölgelere mutex koy.

---

## Rapor İçin Notlar

Sunum/rapor zamanı geldiğinde **sen şu konuları anlatacaksın**:

1. **Mimari kararlar:** Niye thread-per-client, niye SQLite (PostgreSQL değil), niye text protokol
2. **Veri modeli:** Tablolar arası ilişkiler, neden composite index, denormalize counts
3. **Protokol tasarımı:** Niye HTTP değil, raw TCP üstünde komut formatı
4. **Karşılaştığın zorluklar:** Chunked transfer, thread safety, edge cases
5. **Çözümler:** Mutex, prepared statement, recv loop

Bu konuları **sen anlamadan AI'a yazdırırsan sunumda donarsın**. Her özellik bitince:
- AI'a "bu kodun mantığını 30 saniyede özetle" diye sor
- Kafanda canlandır
- Kendine "neden böyle yaptım" diye sor

---

## Hızlı Başlangıç

```
☐ Kurulumun bittiğinden emin ol (echo server testi geçti mi?)
☐ AI sohbeti aç, context dosyalarını yükle:
    - CLAUDE_CONTEXT.md
    - PROTOCOL.md
    - schema.sql
    - SERVER_REHBERI.md (bu dosya)
☐ İlk mesaj:
    "Server tarafını yapıyorum. Aşama 1, İş 1.1 ile başlıyorum:
     util.hpp/cpp dosyaları. Yardım eder misin?"
☐ AI ile birlikte util.cpp'yi yaz, derle, test et, commit et
☐ Sırayla diğer adımlara geç
```

İyi şanslar! Backend'in bel kemiği sensin. 🚀
