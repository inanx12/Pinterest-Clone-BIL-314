# Pinterest-Clone-BIL-314

BİL314 Bilgisayar Ağları dersi final projesi. Kullanıcıların resim ve video
yükleyip paylaşabildiği, Pinterest benzeri bir uygulama.

**Teknolojiler:**
- Server: C++17, POSIX socket, SQLite3, OpenSSL (SHA256)
- Client: C++17, Qt6 Widgets, QTcpSocket
- Protokol: Custom text-based protocol over raw TCP (port 8080)

## Mimari# Pinterest-Clone-BIL-314

[Qt Client] <--TCP/8080--> [C++ Server] <---> [SQLite DB]
|
+---> Disk (media files)

## Build

### Server (Linux)

```bash
sudo apt install build-essential cmake libsqlite3-dev libssl-dev
cd server
mkdir build && cd build
cmake ..
cmake --build .
./server
```

### Client (Windows)

Qt 6.11.0 + MinGW gerekli. Qt Creator ile `client/CMakeLists.txt` aç, build et.

## Protokol

Bkz. [docs/PROTOCOL.md](docs/PROTOCOL.md)

## Veritabanı Şeması

Bkz. [docs/schema.sql](docs/schema.sql)

## Geliştiriciler

- İnan Esen  — 220206016
- İsmail Arslan — yazılacak...

BİL314 Bilgisayar Ağları final projesi.
