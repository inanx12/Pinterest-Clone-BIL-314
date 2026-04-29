# Pinterest-Clone-BIL-314 — Roadmap

Deadline: 10 Mayıs 2026 (LMS)
Bugün: 30 Nisan 2026

## Ekip
- İnan: [SERVER veya CLIENT]
- [Arkadaş]: [SERVER veya CLIENT]

## Gün Gün Plan

### Gün 1-2 (29-30 Nisan) — Setup ✅
- [x] WSL2 + toolchain her iki makinede
- [x] Qt 6.11.0 her iki Windows'ta
- [x] GitHub repo + collaborator
- [x] PROTOCOL.md v1.0
- [x] schema.sql v1.0
- [x] Echo server skeleton

### Gün 3 (1 Mayıs) — DB + İlk komutlar
- [ ] DB wrapper class (db.hpp/cpp)
- [ ] SHA256 + token utility (util.hpp/cpp)
- [ ] Protocol parser (protocol.hpp/cpp)
- [ ] REGISTER handler
- [ ] LOGIN handler
- [ ] Client: Login/Register UI iskeleti

### Gün 4-5 (2-3 Mayıs) — Auth tamamlanır + Media başlar
- [ ] Server: token validation middleware
- [ ] Client: QTcpSocket, sinyal-slot ile cevap parse
- [ ] Server: UPLOAD handler (chunked)
- [ ] Server: LIST handler (basit)
- [ ] Client: dosya seç dialog + upload akışı

### Gün 6-7 (4-5 Mayıs) — Feed + Download
- [ ] Server: DOWNLOAD handler (chunked)
- [ ] Server: PREVIEW handler (resim thumbnail)
- [ ] Client: Pinterest grid (QListView + delegate)
- [ ] Client: Resim/video gösterici
- [ ] Server: DELETE handler
- [ ] Client: silme butonu

### Gün 8 (6 Mayıs) — LIKE + Polish
- [ ] Server: LIKE/UNLIKE
- [ ] Client: like ikonu + count
- [ ] Server: USER_MEDIA (profil)
- [ ] Client: profil sayfası
- [ ] Bug fixes
- [ ] **Hetzner VPS aç + deploy**

### Gün 9 (7 Mayıs) — Entegrasyon + Wireshark
- [ ] End-to-end testler (2 client aynı anda)
- [ ] Wireshark capture al → docs/captures/
- [ ] README'ye build talimatları
- [ ] CHANGE_PASSWORD
- [ ] Edge case'ler (yarım upload, geçersiz token vb.)

### Gün 10 (8 Mayıs) — Doküman + Sunum
- [ ] 8 sayfalık proje raporu
- [ ] 10 slaytlık sunum
- [ ] Mimari diyagram (draw.io)
- [ ] Demo video kaydı (yedek)

### Gün 11 (9 Mayıs) — Buffer
- [ ] Son testler
- [ ] LMS yükleme

### 10 Mayıs — Deadline (24:00)

## Stretch Goals (vakit kalırsa)
- Yorum sistemi (ADD_COMMENT, LIST_COMMENTS, DELETE_COMMENT)
- Video thumbnail (ffmpeg gerektirir)
- Search by username

## Riskler
- **Qt öğrenme süresi** (client tarafı)
- **Chunked file transfer** (UPLOAD/DOWNLOAD'da recv loop)
- **VPS aksaklıkları** (deploy gününde sürpriz çıkabilir)
