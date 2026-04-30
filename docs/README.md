# Pinterest-Clone-BIL-314 — Roadmap v3 (Paralel + Dengelenmiş)

Deadline: 10 Mayıs 2026 (LMS, 24:00)

## Ekip ve İş Bölümü Yaklaşımı
- **İnan:** Auth + Upload/Download + Like/Profile (server+client her ikisi)
- **[Arkadaş]:** Feed + Delete + UX + DevOps (server+client her ikisi)

Her ikisi hem server hem client'a dokunacak. **"Dikey bölünme."** Demo'da "ben şunu yaptım" denebilir, hocaya ikisi de tüm stack'i anlatabilir.

## Faz 1: Tek başına altyapı (Gün 1-2 ✅ + bugün)

**İnan (yalnız):**
- [x] WSL2 + toolchain her iki makinede
- [x] Qt 6.11.0 her iki Windows'ta
- [x] GitHub repo + collaborator
- [x] PROTOCOL.md v1.0
- [x] schema.sql v1.0
- [x] Echo server skeleton
- [ ] util.hpp/cpp (SHA256, random_hex, UUID, timestamp, token_hex)
- [ ] db.hpp/cpp (SQLite wrapper + schema init, schema_path parametre)
- [ ] protocol.hpp/cpp (komut parser + cevap formatlayıcı)

**Arkadaş (kurulumu bitiyor):**
- [x] WSL kurulumu
- [x] Toolchain test (echo server build + nc test)
- [x] AI rehberi + CLIENT temellerini oku
- [ ] Qt6 ile ilk denemeler (3-4 saatlik tutorial)

## Faz 2: Birlikte ortak omurga (Gün 3 = 1 Mayıs)

İkiniz 2-3 saat ortak oturum (Discord ekran paylaş):
- [ ] server.hpp/cpp (TCP listener + accept loop)
- [ ] session.hpp/cpp (per-client thread, komut dispatch)
- [ ] main.cpp güncellenmesi (echo'dan gerçek server'a)
- [ ] **Handler interface kontratı kilitle** (`Response handle_command(...)` imzası — bir daha değişmez)

**Arkadaş ek görev (oturum sonrası, ~2 saat):**
- [ ] **`tools/cli_client.py` yaz** (basit Python: komut yaz, server'a TCP ile gönder, cevap göster, token persist et). Auth/media test sürecinde Qt'yi beklememek için zorunlu. Qt değil, debug aracı.

## Faz 3: Paralel Auth + UI iskelet (Gün 4-5 = 2-3 Mayıs)

**İnan — Auth feature (full stack):**
- [ ] Server: auth.hpp/cpp (REGISTER, LOGIN, validate_token, LOGOUT)
- [ ] Client: NetworkClient sınıfı (QTcpSocket sarmalayıcı)
- [ ] Client: LoginWindow + RegisterWindow
- [ ] Token persistence (QSettings)
- [ ] Auth handler'ları **CLI client ile test** (Qt'yi beklemiyoruz)

**Arkadaş — Client UI temel:**
- [ ] Client: MainWindow iskelet (boş ana pencere, toolbar)
- [ ] Client: Qt öğrenme sürdür
- [ ] CLI client'a iyileştirmeler (renkli output, hata mesajları)
- [ ] Server tarafına dokunmuyor (henüz)

## Faz 3.5: Paralel Media Core (Gün 6-7 = 4-5 Mayıs)

**İnan — Upload/Download feature:**
- [ ] Server: media_upload.cpp (UPLOAD + DOWNLOAD handler, chunked!)
- [ ] Client: UploadDialog (file dialog + progress bar)
- [ ] Client: ImageViewer + VideoPlayer dialog'ları

**Arkadaş — Feed feature:**
- [ ] Server: media_list.cpp (LIST + PREVIEW handler)
- [ ] Client: FeedWidget (Pinterest grid, QListView + delegate)
- [ ] Sıralama: newest / most_liked / most_downloaded

## Faz 4: Paralel Etkileşim + Polish + **VPS Deploy** (Gün 8 = 6 Mayıs)

**İnan — Like + Profile:**
- [ ] Server: media_interaction.cpp (LIKE/UNLIKE/USER_MEDIA)
- [ ] Client: Like ikonu (delegate'e ekle)
- [ ] Client: ProfileWindow

**Arkadaş — Delete + UX + **DevOps**:**
- [ ] Server: DELETE handler (media_interaction.cpp)
- [ ] Server: CHANGE_PASSWORD handler
- [ ] Client: Delete butonu + confirmation dialog
- [ ] Client: Hata mesajları (QMessageBox), loading spinners
- [ ] Client: Logout, settings
- [ ] **Akşam: Hetzner VPS aç + server deploy** (systemd service dahil)
- [ ] **CLI client'tan VPS'e bağlan, smoke test**

> **Not:** VPS deploy bu güne çekildi (önceden Faz 5'teydi). Sebep: Faz 5'i sadece entegrasyon+bug+döküman'a ayırmak istiyoruz. Deploy'da sürpriz çıkarsa, gün 9 yedek olur. CHANGE_PASSWORD da arkadaşa kaydı (denge için).

## Faz 5: Entegrasyon + Doküman başla (Gün 9 = 7 Mayıs)

İkisi birlikte 6-8 saat:
- [ ] End-to-end testler (2 client aynı anda VPS'e bağlı)
- [ ] Edge case'ler: yarım upload, geçersiz token, bozuk komut, büyük dosya
- [ ] **Wireshark capture al → docs/captures/**
- [ ] README'ye build talimatları
- [ ] Bug avı (yeni özellik EKLEMEYIN)
- [ ] **Akşam: Demo video kaydı** (sistem en stabil halindeyken — yarın bozulursa bu video LMS'e gider)
- [ ] Mimari diyagram başla (draw.io)

## Faz 6: Doküman + Sunum (Gün 10 = 8 Mayıs)

**İnan:**
- [ ] Rapor server bölümleri (mimari, protokol, DB)
- [ ] Sunum server slaytları

**Arkadaş:**
- [ ] Rapor client bölümleri (Qt mimarisi, async UI, grid)
- [ ] Sunum client slaytları

**Birlikte:**
- [ ] Mimari diyagram bitir (draw.io)
- [ ] Rapor giriş/sonuç bölümleri
- [ ] 8 sayfa sınırını kontrol et
- [ ] 10 slayt sınırını kontrol et
- [ ] Sunum provası (rol değiştirerek — sen client'ı, o server'ı anlatsın)

## Faz 7: Buffer + Teslim (Gün 11 = 9 Mayıs)

- [ ] Son testler
- [ ] LMS'e yükle: rapor + sunum + GitHub link
- [ ] **YENİ ÖZELLİK YOK, KOD DEĞİŞİKLİĞİ YOK**

## 10 Mayıs — Deadline (24:00)

---

## Stretch Goals (vakit kalırsa, atlanabilir)
- [ ] Yorum sistemi (ADD_COMMENT, LIST_COMMENTS, DELETE_COMMENT)
- [ ] Video thumbnail (ffmpeg gerektirir, server tarafında)
- [ ] Search by username
- [ ] Multi-language (Türkçe/İngilizce UI toggle)

## Riskler ve Karşı Önlemler

| Risk | Olasılık | Önlem |
|---|---|---|
| Qt öğrenme süresi (ikisi için) | Yüksek | Faz 1'de arkadaş 3-4 saat tutorial; CLI client ile Qt-bağımsız test |
| Chunked file transfer karmaşası | Orta | recv/send döngüsü AI ile birlikte yaz, CLI'dan erken test |
| VPS deploy sürprizleri | Orta | Faz 4 akşamına çekildi, gün 9 yedek |
| Merge conflict (paralel çalışma) | Yüksek | Dosya bazlı bölünme + Discord sync |
| Arkadaş gecikmeli kurulum | Onaylandı | Faz 1'de İnan altyapı atıyor |
| Sistem son gün bozulur | Düşük | Demo video Faz 5 akşamı (gün 9) çekildi |
| Hocaya cevap verememe | Orta | Faz 6'da rol değişerek prova |

## Çakışma Önleme Kuralları

1. **Dosya bölünmesi:** `media.cpp` yerine `media_upload.cpp` + `media_list.cpp` + `media_interaction.cpp`
2. **Ortak dosyalara dokunmadan haber ver:** main.cpp, session.cpp, CMakeLists.txt, PROTOCOL.md, schema.sql
3. **Branch + PR:** Her büyük iş feature branch'ta, PR ile merge
4. **Günlük sync:** 09:00 standup (5 dk) + 21:30 sync (15 dk)
5. **Bilmediğin koda dokunma:** Önce diğerine sor, anlat, sonra dokun

## v2'den v3'e Değişiklikler

- **CLI test client eklendi** (Faz 2 sonu, arkadaşın görevi). Auth/media test ederken Qt'yi beklemek zorunda kalmıyoruz.
- **VPS deploy Faz 5'ten Faz 4'e çekildi.** Gün 9 sadece entegrasyon+döküman için kaldı. Deploy sürprizi gün 9'a sızmıyor.
- **CHANGE_PASSWORD İnan'dan arkadaşa kaydı.** Faz 4 yükünü dengeledi.
- **Demo video Faz 6'dan Faz 5 akşamına çekildi.** Sistem en stabil olduğu noktada kaydedilir.
- **Faz 6'dan demo video kaldırıldı** (artık Faz 5'te).
