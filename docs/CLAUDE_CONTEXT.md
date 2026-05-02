# Proje: Pinterest-Clone-BIL-314

BİL314 Bilgisayar Ağları final projesi. Deadline: 10 Mayıs 2026 (LMS).
Hoca raw TCP istiyor (HTTP yasak), C/C++ ile, packet log isteyebilir.

## Ekip
- İnan (server tarafı) — github: inanx12
- İsmail (Client tarafı)

## Stack
- **Server:** C++17, POSIX socket, std::thread per-client, SQLite3, OpenSSL (SHA256)
  - WSL2 Ubuntu 22.04 üzerinde geliştirme, deploy hedefi: Linux VPS (Hetzner muhtemel)
  - Build: CMake
- **Client:** C++17, Qt6 Widgets (Qt 6.11.0 MinGW 64-bit), QTcpSocket, QMediaPlayer
  - Windows native, E:\Dev\ altında ikinci klon
- **DB:** SQLite, schema HENÜZ YAZILMADI (sıradaki iş)
- **Protokol:** Raw TCP üstünde text-based, port 8080, dosya transferleri binary
  - Tam spec: docs/PROTOCOL.md (v1.0)

## Şu ana kadar yapılanlar
- [x] GitHub repo (private): inanx12/Pinterest-Clone-BIL-314
- [x] PROTOCOL.md v1.0 yazıldı
- [x] Klasör yapısı: server/, client/, docs/, .gitignore
- [x] WSL2 + toolchain her iki makinemde (masaüstü + laptop) hazır
- [x] Qt 6.11.0 her iki Windows'umda hazır (E:\QT)
- [x] server/src/main.cpp: TCP echo skeleton (v0.1) çalışıyor
- [x] netcat ile localhost:8080 üzerinde echo testi başarılı

## Sıradaki adımlar
- [x] Arkadaşın WSL kurulumu (kendisi yapıyor)
- [x] docs/schema.sql: users, sessions, media, likes, (comments) tabloları
- [x] İş bölümü kararı (kim server kim client)
- [ ] Server: REGISTER + LOGIN handler (SHA256 + token)
- [ ] Client: Qt login ekranı + QTcpSocket bağlantısı
- [ ] Gün 7-8: VPS deploy
- [ ] Gün 10: Doküman + sunum

## Önemli kararlar / kısıtlar
- Yorum sistemi STRETCH GOAL (vakit kalırsa)
- Video için thumbnail yok, sadece resim için
- Token süresiz, multi-device
- Maksimum dosya: resim 20MB, video 100MB
- Public + private görünürlük seçeneği var
- Tailscale/ngrok şifreli olduğu için KULLANILMAYACAK (hoca packet log isteyebilir)

## Kullanıcı tercihleri
- Türkçe konuş, dürüst ve kısa cevap, gereksiz yağcılık yok
- Risk/zorluk varsa baştan söyle, sürpriz olmasın
- Öğrenci, sıkışık deadline, pratik çözüm odaklı
