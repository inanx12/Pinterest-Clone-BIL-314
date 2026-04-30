# CLIENT Tarafı Seçtiysen — Yol Haritası

Selam! Client tarafını seçtin. Bu rehber sana **ne yapacağını, hangi sırayla, AI'a ne sorman gerektiğini** anlatıyor.

---

## Client Ne İşe Yarıyor?

Sen **kullanıcının gördüğü** kısmı yapıyorsun. Pinterest uygulaması gibi düşün — login ekranı, ana sayfa, resimler, video oynatıcı. Ne yapacak:

- Kullanıcıya login/register ekranı göster
- Sunucuya TCP bağlantı kur (port 8080)
- Komutları gönder (REGISTER, LOGIN, UPLOAD vs.)
- Sunucudan gelen cevapları parse et
- Resimleri/videoları ekrana getir
- Pinterest grid layout
- Upload progress bar
- Like/silme butonları

**Server'la fark:** Sen "ne gözüküyor"u yaparsın, server "ne işleniyor"u.

---

## Çalışma Ortamın

- **OS:** Windows (native, WSL'de değil)
- **Dil:** C++17
- **GUI Framework:** Qt6 Widgets (Qt 6.11.0 MinGW 64-bit)
- **IDE:** Qt Creator
- **Network:** QTcpSocket (Qt'nin kendi socket'i, asenkron)
- **Multimedia:** QMediaPlayer + QVideoWidget

Hepsi `C:\Qt` (veya `E:\Qt`) altında zaten kurulu.

---

## Qt Hakkında Hızlı Bilgi (Önce Bunu Oku)

### Qt'nin temel kavramları

**1. Widget = UI öğesi**
Buton, textbox, label, pencere — hepsi widget. `QPushButton`, `QLineEdit`, `QLabel`, `QWidget` gibi.

**2. Layout = düzen**
Widget'ları yerleştirme. `QVBoxLayout` (dikey), `QHBoxLayout` (yatay), `QGridLayout` (ızgara).

**3. Sinyal-Slot = event sistemi**
Bir şey olunca (örn butona tıklandı), başka bir fonksiyon çalışır:

```cpp
connect(loginButton, &QPushButton::clicked, this, &MainWindow::onLoginClicked);
//      ^kim?         ^ne sinyali?           ^kim alacak  ^hangi fonksiyon
```

Bu satır: "loginButton tıklanınca bu pencerenin `onLoginClicked()` fonksiyonu çalışsın".

**4. Asenkron network**
QTcpSocket cevabı **hemen** vermez. Cevap geldiğinde `readyRead` sinyali tetiklenir, sen ona bağladığın slot'ta cevabı okursun. Bunu kavramak Qt'nin en kafa karıştırıcı kısmıdır.

### Qt Designer = Form Builder (drag-drop)

Qt Creator'da `.ui` dosyası açtığında **görsel editor** gelir. Widget'ları sürükle-bırak ekler, kod yazmazsın. Sonra koddan `ui->loginButton` gibi erişirsin.

**Bu seni çok rahatlatacak.** Login formu için 5 dakika sürükle-bırak iş, sıfır kod.

### Öğrenmen gereken (3-4 saatlik yatırım)

1. Qt Designer'da form yapma (sürükle-bırak)
2. Sinyal-slot mekanizması (5 farklı yerde göreceksin, ezberleyeceksin)
3. QTcpSocket asenkron kullanımı
4. QListView + custom delegate (Pinterest grid için)

---

## Klasör Yapısı

```
client/
├── CMakeLists.txt              ← Qt build config
├── src/
│   ├── main.cpp                ← Qt app girişi
│   ├── mainwindow.hpp/cpp      ← Ana pencere
│   ├── mainwindow.ui           ← Designer dosyası (XML)
│   ├── loginwindow.hpp/cpp     ← Login penceresi
│   ├── loginwindow.ui
│   ├── feedwidget.hpp/cpp      ← Pinterest grid
│   ├── networkclient.hpp/cpp   ← QTcpSocket sarmalayıcı
│   ├── mediaitem.hpp/cpp       ← Tek bir post (image/video tile)
│   └── uploaddialog.hpp/cpp    ← Dosya yükleme dialog'u
└── build/                      ← cmake derler buraya
```

---

## Yol Haritası — Hangi Sırayla Yapacaksın

### Aşama 0: Qt Öğrenme (Gün 3 — 3-4 saat)

**Kod yazmadan önce Qt'yi tanı.**

#### İş 0.1: Resmi tutorial (1.5 saat)

YouTube'dan "Qt 6 C++ Hello World" tarzı bir tutorial bul, izle ve takip et. Türkçe veya İngilizce fark etmez. **Sadece izleme, gerçekten yap.**

Hedefin: Boş bir pencere açan basit bir Qt uygulaması yazmak ve çalıştırmak.

#### İş 0.2: Login formu denemesi (1.5 saat)

**Qt Designer ile:**
1. Qt Creator → File → New → Application (Qt) → Qt Widgets Application
2. `mainwindow.ui` aç
3. Sol panelden sürükle bırak:
   - 2 adet QLabel ("Username", "Password")
   - 2 adet QLineEdit
   - 1 adet QPushButton ("Login")
4. Üstüne sağ tık → Lay Out → Lay Out in a Form Layout
5. Run et, formu gör

**Sonra koda gir:**
6. Login butonuna sağ tık → "Go to slot" → "clicked()" → tıkla
7. Otomatik açılan fonksiyona şunu yaz:
   ```cpp
   void MainWindow::on_loginButton_clicked() {
       QString username = ui->usernameEdit->text();
       QString password = ui->passwordEdit->text();
       QMessageBox::information(this, "Test", "Username: " + username);
   }
   ```
8. Run et, butona tıkla, popup'ta username gözükmeli.

**Bu deneyimi yaşadığın an Qt'yi öğrenmişsin demektir.** Gerisi aynı mantığın tekrarı.

#### İş 0.3: AI'a sor

```
[CONTEXT_DOSYALARI yükle]

Qt6 Widgets'ı yeni öğreniyorum. Aşağıdaki temel kavramları kısa ve örnekli açıkla:
1. Sinyal-slot nedir, syntax nasıl
2. QTcpSocket'in asenkron yapısı (readyRead sinyali)
3. QListView'a custom delegate yazmak
4. parent/child ownership (memory leak'ten korunma)

Tek tek açıkla, her birine 5-10 satırlık bir örnek kod ekle.
```

### Aşama 1: Network Katmanı (Gün 3-4)

#### İş 1.1: NetworkClient sınıfı (2-3 saat)

**Bu sınıf tüm server iletişimini yönetir.** UI'dan ayrılmış olsun, daha temiz olur.

**Sorumlulukları:**
- QTcpSocket sahibi
- `connectToServer(host, port)` metodu
- `sendCommand(QString)` metodu
- `responseReceived(QString)` sinyali (her cevap için)
- `connected`, `disconnected`, `error` sinyalleri

**Önemli:** TCP'de cevap **birkaç paket** olarak gelebilir. Yani `readyRead` tetiklendiğinde tam cevap gelmemiş olabilir. Buffer tutman lazım.

**AI'a sor:**
```
[CONTEXT_DOSYALARI yükle]

Qt6 ile NetworkClient sınıfı yazmak istiyorum. Server PROTOCOL.md'de tanımlı,
text-based, satır sonu \n.

Sınıf:
- QTcpSocket'i sarsın
- connectToServer(host, port) metodu
- sendCommand(QString cmd) metodu - tek satır gönderir
- Sinyaller: connected, disconnected, errorOccurred(QString)
- Sinyaller: textResponseReceived(QString line) - her tam satır geldiğinde

Kritik: readyRead'de gelen veriyi buffer'a ekle, \n karakterine göre satırlara böl.
Eksik satır kalırsa buffer'da bekletip sonraki readyRead'i bekle.

Binary download için ayrı bir mekanizma lazım ama şimdilik sadece text kısmını yaz.

Hpp ve cpp dosyalarını ayrı ayrı ver, her metoda kısa açıklama ekle.
```

#### İş 1.2: Test (1 saat)

main.cpp'ye geçici test:
```cpp
NetworkClient client;
QObject::connect(&client, &NetworkClient::textResponseReceived,
                 [](QString line) { qDebug() << "Got:" << line; });
client.connectToServer("localhost", 8080);
client.sendCommand("REGISTER testuser pass123");
```

İnan'ın server'ını çalıştır (WSL'de), test et. "OK <token>" gelmeli.

### Aşama 2: Login Ekranı (Gün 4)

#### İş 2.1: LoginWindow (2 saat)

**Tasarım (Qt Designer ile):**
- Logo/başlık (QLabel)
- Username field (QLineEdit)
- Password field (QLineEdit, echoMode = Password)
- Login button (QPushButton)
- Register button (QPushButton)
- Status label (hata mesajları için)

**Mantık:**
- Login butonu → `LOGIN <user> <pass>\n` gönder
- Cevap "OK" başlıyorsa → token'ı sakla, ana ekrana geç
- "ERR" başlıyorsa → hata mesajını göster
- Register butonu → `REGISTER <user> <pass>\n` gönder, aynı mantık

**AI'a sor:**
```
LoginWindow sınıfını yazmak istiyorum. Bir önceki adımda yazdığım NetworkClient'i kullanacak.

UI elemanları (Designer ile yapacağım):
- usernameEdit (QLineEdit)
- passwordEdit (QLineEdit, echo password)
- loginButton, registerButton (QPushButton)
- statusLabel (QLabel)

Akış:
1. Login basıldığında: input validate, networkClient.sendCommand(...)
2. textResponseReceived sinyalini dinle
3. "OK" ile başlıyorsa token'ı sakla, accept() çağır (modal kapansın)
4. "ERR" ile başlıyorsa statusLabel'a yaz

Token'ı QSettings ile diske kaydet, bir sonraki açılışta auto-login olsun.

Hpp + cpp + ui dosyalarını ver.
```

#### İş 2.2: Otomatik login

Token diske kaydedildiğinden, uygulama açılışında:
- QSettings'ten token oku
- Varsa direkt ana ekrana geç (login skip)
- Yoksa LoginWindow göster

### Aşama 3: Ana Pencere + Feed (Gün 5-6)

#### İş 3.1: MainWindow iskeleti (1 saat)

**Tasarım:**
- Üst toolbar: "Upload", "Profile", "Logout" butonları
- Ortada FeedWidget (Pinterest grid)
- Sağ üstte sıralama dropdown (newest / most_liked / most_downloaded)

#### İş 3.2: FeedWidget — Pinterest Grid (3-4 saat — EN ZOR KISIM)

**Yaklaşım 1 (Basit, önerim):** `QListView` + `QStandardItemModel` + custom `QStyledItemDelegate`

**Yaklaşım 2 (Daha güzel):** `QGridLayout` içinde dinamik widget ekleme

**AI'a sor (basit yaklaşım):**
```
Pinterest tarzı grid layout istiyorum. Her item bir image/video preview ve altında küçük metadata (owner, like count).

Yaklaşım: QListView + custom delegate.
- ListView mode: IconMode
- Resize mode: Adjust
- View mode: Wrap on resize
- 3 sütunlu grid hissi

Custom delegate:
- Image kısmı (QPixmap, scaled to width)
- Owner ismi alt satırda
- Like ikonu + sayı

Veriyi nasıl alacağım:
1. Açılışta networkClient.sendCommand("LIST <token> newest 0 20")
2. Cevap JSON, parse et
3. Her item için PREVIEW <token> <id> ile thumbnail çek (binary)
4. Pixmap'i model'e ekle, view kendiliğinden render eder

Önce model + view skeleton yaz, sonra delegate. Adım adım gidelim.
```

#### İş 3.3: Resim/video gösterici dialog (2 saat)

Item'a tıklandığında:
- Resim ise → QDialog + QLabel + scaled QPixmap
- Video ise → QDialog + QMediaPlayer + QVideoWidget

DOWNLOAD komutu ile tam dosyayı çek, geçici dosyaya yaz, oynat.

**AI'a sor:**
```
Bir media item'a tıklandığında açılan dialog yazmak istiyorum.

İki tip var:
- image: QLabel'da göster, fit to dialog, aspect ratio koru
- video: QMediaPlayer + QVideoWidget, play/pause/seek bar

İndirme akışı:
1. networkClient.sendCommand("DOWNLOAD <token> <media_id>")
2. Cevap binary geliyor: "OK <type> <size>\n" + size byte
3. Geçici dosyaya yaz (QStandardPaths::TempLocation)
4. QPixmap.load(path) veya QMediaPlayer.setSource(path)

Binary cevap için NetworkClient'a binary mode eklenmesi lazım. 
Bunu nasıl yaparım? Şu anki NetworkClient sadece text alıyor.
```

### Aşama 4: Upload (Gün 6-7)

#### İş 4.1: UploadDialog (2 saat)

**Tasarım:**
- "Dosya Seç" butonu (QFileDialog açar)
- Önizleme (resim ise küçük thumbnail, video ise dosya adı)
- Visibility radio: Public / Private
- Upload butonu
- Progress bar

**Mantık:**
1. Dosya seç → boyut kontrol (20MB image / 100MB video)
2. Upload basıldığında: `UPLOAD <token> <type> <visibility> <filename> <size>\n`
3. "READY\n" cevabı bekle
4. Dosyayı chunk chunk gönder, progress bar güncelle
5. "OK <media_id>\n" geldiğinde başarı mesajı, dialog kapat
6. Feed'i yenile

**AI'a sor:**
```
UploadDialog yazmak istiyorum. Akış PROTOCOL.md 4.4'e göre.

1. QFileDialog::getOpenFileName ile dosya seç
2. Uzantıdan tip belirle (jpg/png → image, mp4/mov/mkv → video)
3. Boyut kontrol et
4. Visibility için 2 QRadioButton
5. Upload butonuna basınca:
   a. UPLOAD komutu gönder
   b. textResponseReceived'i dinle: "READY" geldiğinde dosyayı chunk olarak gönder
   c. Her chunk'tan sonra QProgressBar güncelle
   d. "OK <id>" cevabı için tekrar dinle, dialog'u accept et

Chunked send: 64KB chunk uygun mu? QTcpSocket'te bunu nasıl yapayım?
```

#### İş 4.2: Upload sonrası feed yenileme

Upload başarılı olunca FeedWidget'a "yenile" sinyali gönder, LIST'i tekrar çek.

### Aşama 5: Like, Delete, Profile (Gün 7-8)

#### İş 5.1: Like butonu

Item delegate'inde küçük bir kalp ikonu, tıklanınca `LIKE` veya `UNLIKE` gönder, count güncelle.

#### İş 5.2: Delete butonu

Item'ın sağ köşesinde `x` ikonu (sadece kendi item'larında görünsün). Tıklanınca confirmation dialog, `DELETE` gönder, item'ı listeden kaldır.

#### İş 5.3: Profile sayfası

Toolbar'da "Profile" butonu → ProfileWindow:
- USER_MEDIA komutu ile kendi medyalarını listele
- Aynı FeedWidget'ı tekrar kullan, sadece source farklı

### Aşama 6: Polish (Gün 8-9)

- Hata mesajları için QMessageBox dialog'ları
- Loading spinner (long operations için)
- Kısayol tuşları (Ctrl+U → upload, Ctrl+L → logout vs.)
- About dialog (proje bilgisi)
- Logo/icon
- Pencere ikonu, başlık çubuğu

---

## CMakeLists.txt (Qt için)

```cmake
cmake_minimum_required(VERSION 3.20)
project(pinterest_client CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets)

add_executable(client
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    src/loginwindow.cpp
    src/loginwindow.ui
    # ... diğerleri
)

target_link_libraries(client PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)
```

İlk yazımda AI'a:
```
Qt 6.11 + MinGW 64-bit kullanıyorum. CMakeLists.txt'i hazırla, gerekli
modülleri (Widgets, Network, Multimedia) ekle. Qt'nin AUTO MOC/UIC/RCC
ayarlarını aç.
```

---

## Build Etme

Qt Creator'dan ya da komut satırından:

```powershell
cd C:\Dev\Pinterest-Clone-BIL-314\client
mkdir build
cd build
& "C:\Qt\6.11.0\mingw_1310_64\bin\qt-cmake.bat" ..
mingw32-make -j8
.\client.exe
```

Veya **çok daha kolayı**: Qt Creator'da projeyi aç, sol alttaki yeşil ▶ butonuna bas. Hepsi otomatik.

---

## Sıkça Karşılaşacağın Sorunlar

### "Qt MOC error"
`Q_OBJECT` macro kullandın ama AUTOMOC açık değil. CMakeLists'te `set(CMAKE_AUTOMOC ON)` olmalı.

### "QApplication: No such file or directory"
`find_package(Qt6 REQUIRED COMPONENTS Widgets)` eksik veya Qt path tanımlı değil.

### "QTcpSocket cevabı eksik geliyor"
`readyRead` sinyali her byte için tetiklenebilir. Buffer kullan, `\n` görene kadar bekle.

### "Pencere açılmıyor"
`main.cpp`'de `QApplication app(argc, argv)` ve `app.exec()` var mı? `MainWindow w; w.show();` arada olmalı.

### "Türkçe karakter bozuk"
`setlocale(LC_ALL, "tr_TR.UTF-8");` veya kaynak dosyaları UTF-8 kaydet.

---

## Test Stratejisi

Her özellik için:
1. **Server'sız test:** UI tek başına açılıp doğru gözüküyor mu? (Mock data ile)
2. **Server'lı test:** İnan'ın server'ına bağlanıp gerçek komutu gönder
3. **Hata senaryoları:** Server kapalıyken ne oluyor? Yanlış şifre girince? 

İnan'ın server'ı çalışmasa bile **UI üstünde çalışabilirsin** — tüm ekranları, layout'ları yapabilirsin. Sonra bağlantıyı eklersin. Bu çok güzel bir özellik, paralel çalışmayı kolaylaştırır.

---

## Rapor İçin Notlar

Sunum/rapor zamanı geldiğinde **sen şu konuları anlatacaksın**:

1. **Qt seçim sebebi:** Cross-platform, hazır widget'lar, Multimedia desteği
2. **Asenkron iletişim:** QTcpSocket sinyal-slot ile, UI bloklamadan
3. **Pinterest grid:** QListView + custom delegate yaklaşımı
4. **Upload akışı:** Chunked transfer, progress bar entegrasyonu
5. **Karşılaştığın zorluklar:** TCP buffer parsing, async cevap timing, QMediaPlayer codec'leri

Demo sırasında parlayan kısım sensin (görsel demo). İyi yap!

---

## Hızlı Başlangıç

```
☐ Kurulumun bittiğinden emin ol (Qt Creator + WSL toolchain)
☐ Önce Qt öğren (Aşama 0, 3-4 saat)
☐ AI sohbeti aç, context dosyalarını yükle:
    - CLAUDE_CONTEXT.md
    - PROTOCOL.md
    - schema.sql
    - CLIENT_REHBERI.md (bu dosya)
☐ İlk mesaj:
    "Client tarafını yapıyorum, Qt'yi yeni öğreniyorum.
     Aşama 1, İş 1.1 ile başlıyorum: NetworkClient sınıfı.
     Önce Qt'nin sinyal-slot ve QTcpSocket asenkron mekanizmasını
     örneklerle açıklayabilir misin?"
☐ Anlayınca kod yazmaya başla
☐ Sırayla diğer adımlara geç
```

İyi şanslar! Görsel sunum sensin, parlayacak olan da sen. 🎨
