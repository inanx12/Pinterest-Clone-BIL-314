# Pinterest-Clone-BIL-314 — Roadmap

**Deadline:** 10 Mayıs 2026 (LMS, 24:00)
**Bugün:** 1 Mayıs 2026

## Ekip

| Kişi | Rol | Klasör |
|------|-----|--------|
| **İnan** | 🖥️ SERVER (backend) | `server/` |
| **[Arkadaş]** | 🎨 CLIENT (frontend) | `client/` |

**Ortak alan:** `docs/`, `common/` (PR ile değiştirilir, önce konuşulur)

---

## Gün Gün Plan — Paralel Çalışma

### Gün 1-2 (29-30 Nisan) — Setup ✅ TAMAMLANDI

- [x] WSL2 + toolchain (her iki makinede + her iki tarafta)
- [x] Qt 6.11.0 (Windows tarafları)
- [x] GitHub repo + collaborator
- [x] `docs/PROTOCOL.md` v1.0
- [x] `docs/schema.sql` v1.0
- [x] Echo server skeleton
- [x] `common/constants.hpp` (ortak sabitler)
- [x] Server src iskeleti (.hpp dosyaları)
- [x] Client src iskeleti (.hpp/.cpp/.ui dosyaları)
- [x] Doküman seti (REHBER dosyaları)

---

### Gün 3 (1 Mayıs) — Yardımcı Modüller + Network Katmanı

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ `util.cpp` — sha256_hex, random_hex, uuid_hex, now_unix | ☐ Qt6 Widgets temel öğrenme (3-4 saat tutorial) |
| ☐ `db.cpp` — SQLite wrapper, schema init, PRAGMA'lar | ☐ Qt Designer ile basit form denemesi |
| ☐ `protocol.cpp` — komut parser, cevap formatlayıcılar | ☐ `networkclient.cpp` — QTcpSocket sarmalayıcı |
| ☐ Manuel test: util fonksiyonları, db init, parser | ☐ Test: localhost echo server'a "merhaba" gönder |

**Gün 3 sonu hedef:** İnan'ın yardımcı modülleri ve arkadaşın network katmanı çalışıyor olmalı.

---

### Gün 4 (2 Mayıs) — Auth (REGISTER + LOGIN)

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ `auth.cpp` — register_user (validation + INSERT) | ☐ `loginwindow.ui` — Designer ile form |
| ☐ `auth.cpp` — login_user (hash kontrol + token üret) | ☐ `loginwindow.cpp` — sinyal-slot, network bağlantısı |
| ☐ `auth.cpp` — validate_token | ☐ Token saklama (`QSettings`) |
| ☐ `auth.cpp` — logout | ☐ Login → MainWindow geçişi |
| ☐ `session.cpp` — dispatch loop (REGISTER, LOGIN, LOGOUT) | ☐ Hata mesajları gösterimi (QMessageBox) |

**Gün 4 sonu hedef:** Server REGISTER/LOGIN/LOGOUT cevaplıyor; Client formu hazır ama henüz entegre değil.

---

### Gün 5 (3 Mayıs) — ⭐ İLK ENTEGRASYON ⭐

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ `media.cpp` — UPLOAD handler (chunked recv) | ☐ İnan'ın server'ına gerçek REGISTER/LOGIN |
| ☐ Boyut/format/MIME kontrolü | ☐ Token'ın kalıcı saklanması test |
| ☐ media tablosuna INSERT, dosya yazma | ☐ MainWindow iskelet (toolbar + ana alan) |
| ☐ Hata yönetimi (yarım upload temizliği) | ☐ Logout flow |

**Gün 5 sonu hedef:** Client gerçek server'a bağlanıp REGISTER/LOGIN yapabiliyor. İlk kritik milestone.

---

### Gün 6 (4 Mayıs) — Feed + LIST

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ `media.cpp` — LIST handler (sort: newest/most_liked/most_downloaded) | ☐ `feedwidget.cpp` — QListView + StandardItemModel |
| ☐ JSON output (elle string concat veya nlohmann/json) | ☐ Custom delegate (resim + owner + like sayısı) |
| ☐ Sayfalama (offset, limit) | ☐ Sıralama dropdown (UI) |
| ☐ liked_by_me bilgisi | ☐ Feed'i LIST komutu ile doldur |
| ☐ Public/private filtreleme | ☐ Boş feed durumu |

**Gün 6 sonu hedef:** Feed gerçek veriyle dolup ekranda gözüküyor.

---

### Gün 7 (5 Mayıs) — DOWNLOAD/PREVIEW + Upload UI

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ `media.cpp` — DOWNLOAD (chunked send) | ☐ `uploaddialog.cpp` — file picker, visibility seçimi |
| ☐ `media.cpp` — PREVIEW (resim için thumbnail) | ☐ Chunked upload + progress bar |
| ☐ download_count++ | ☐ Resim gösterici dialog (QLabel + QPixmap) |
| ☐ Private kontrolü (sadece owner) | ☐ Video oynatıcı (QMediaPlayer + QVideoWidget) |
| | ☐ Feed'e thumbnail çekme (PREVIEW kullan) |

**Gün 7 sonu hedef:** Upload + view + download çalışıyor. Tam akış.

---

### Gün 8 (6 Mayıs) — LIKE + Profile + Polish

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ `media.cpp` — LIKE/UNLIKE (atomic update) | ☐ Like ikonu + count (delegate) |
| ☐ `media.cpp` — DELETE | ☐ Silme butonu (kendi medyaları) |
| ☐ `media.cpp` — USER_MEDIA | ☐ Profil sayfası (kullanıcı kendi medyaları) |
| ☐ CHANGE_PASSWORD | ☐ UI polish (renkler, fontlar, ikonlar) |
| ☐ **Hetzner CX22 VPS aç + deploy** | ☐ Pencere ikonu, başlık, "About" |

**Gün 8 sonu hedef:** Tüm fonksiyonlar bitti, VPS canlı.

---

### Gün 9 (7 Mayıs) — Entegrasyon + Wireshark

| 🖥️ SERVER (İnan) | 🎨 CLIENT (Arkadaş) |
|---|---|
| ☐ Edge case'ler (yarım upload, geçersiz token, vs.) | ☐ Edge case'ler (server kapalı, timeout, vs.) |
| ☐ Server log'u → docs/server-log.txt | ☐ Loading spinner, hata mesajları |
| ☐ SQL injection testi (prepared stmt doğrula) | ☐ Kısayol tuşları (Ctrl+U upload, vs.) |
| ☐ **Wireshark capture** → docs/captures/ | ☐ İki client aynı anda → çakışma var mı |

**Beraber:**
- ☐ End-to-end test senaryoları (5-6 farklı use case)
- ☐ README'ye build talimatları (her iki taraf için)
- ☐ docs/screenshots/ — sunum için ekran görüntüleri

---

### Gün 10 (8 Mayıs) — Doküman + Sunum

**Doküman dağılımı (8 sayfa):**

| 🖥️ İnan yazar | 🎨 Arkadaş yazar |
|---|---|
| Sistem mimarisi + diyagram | Kullanılan metodoloji + Qt seçimi |
| Server tarafı detayları (threading, DB, SQLite) | Client tarafı detayları (Qt Widgets, sinyal-slot) |
| Protokol özeti | Kullanıcı arayüzü tasarımı |
| Karşılaştığı zorluklar (chunked transfer vs.) | Karşılaştığı zorluklar (Qt öğrenme, async UI) |
| | Kapak + ekip + giriş + sonuç (ortak yazılır) |

**Sunum dağılımı (10 slayt):**
- 1-2: Beraber (kapak, problem, mimari)
- 3-5: İnan (server, protokol, DB)
- 6-8: Arkadaş (client, UI, demo)
- 9-10: Beraber (zorluklar, sorular)

**Diyagramlar:** [draw.io](https://app.diagrams.net) ile mimari diyagramı çiz.

---

### Gün 11 (9 Mayıs) — Buffer

- [ ] Son testler (her iki tarafın da çalıştığını doğrula)
- [ ] LMS yükleme (proje tanımlama + kaynak kod GitHub linki + sunum)
- [ ] Demo videosu kaydet (yedek olarak — sunum günü internet/VPS sorun çıkarsa)
- [ ] Hocaya teslim öncesi son git push'lar

### 10 Mayıs (Cumartesi) — DEADLINE 24:00

---

## Stretch Goals (vakit kalırsa)

| Özellik | Server tarafı | Client tarafı |
|---|---|---|
| Yorum sistemi | ADD_COMMENT, LIST_COMMENTS, DELETE_COMMENT | Yorum görüntüleme + ekleme UI |
| Video thumbnail | ffmpeg ile preview üretme | Feed'de video preview |
| Search | SEARCH komutu (LIKE %query%) | Search bar |

---

## Riskler

| Risk | Etkilenen | Önlem |
|---|---|---|
| Qt öğrenme süresi uzayabilir | Arkadaş | Gün 3'te tutorial bitsin, takılırsa İnan'dan yardım |
| Chunked file transfer bug'ları | İnan | Erken test, küçük dosyalardan büyüğe |
| VPS deploy aksaklıkları | Beraber | Gün 8 yedek olarak Gün 9'a alınabilir |
| Entegrasyon Gün 5'te tutmazsa | Beraber | Hemen senkron oturum, ortak debug |

---

## Workflow Hatırlatması

**Her oturum başında:**
```bash
git pull
```

**Bittiğinde:**
```bash
git add .
git commit -m "[Server/Client] anlamlı mesaj"
git push
```

**Asla yapma:**
- ❌ Karşı klasöre dokunma (server kişisi `client/`'a, client kişisi `server/`'a)
- ❌ `docs/` veya `common/` içinde değişiklik öncesi konuşmadan push
- ❌ `git push --force`
- ❌ Bozuk kod push (önce derlenmesini test et)

**Her sabah 10 dakika senkron:**
- Dün ne yaptım
- Bugün ne yapacağım
- Engelim/sorum var mı

---

## Mevcut Durum (1 Mayıs Akşamı)

```
🟢 Setup tamamlandı (Gün 1-2)
🟡 Gün 3 başlıyor — yardımcı modüller + network katmanı
⏳ Gün 5'te ilk entegrasyon hedefi
🎯 Gün 8'de tüm fonksiyonlar
🚀 Gün 10'da doküman + sunum
```
