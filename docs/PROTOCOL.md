# Pinterest-Clone Protokolü v1.0

BİL314 Bilgisayar Ağları – Final Projesi
Server-Client haberleşme protokolü tanımı.

## 1. Genel Kurallar

- **Transport:** TCP, port `8080`
- **Bağlantı modeli:** Persistent — client login olduktan sonra socket açık kalır, tüm komutlar aynı bağlantı üzerinden gider. Bağlantı koparsa client otomatik yeniden bağlanır ve kayıtlı token ile auth olur.
- **Encoding:** Komutlar UTF-8 text, satır sonu `\n`. Dosya payload'ları binary.
- **Komut formatı:** `<COMMAND> <arg1> <arg2> ... \n` — argümanlar boşlukla ayrılır. Boşluk içeren argüman olamaz (ileride gerekirse base64 veya quote eklenir, v1'de yasak).
- **Cevap formatı:** `OK [data...]\n` veya `ERR <code> <message>\n`
- **Token:** Login sonrası verilen 32 karakterlik random hex string. Süresiz, sadece logout veya şifre değişiminde geçersiz olur. Bir kullanıcının aynı anda birden fazla geçerli token'ı olabilir (multi-device).

## 2. Hata Kodları

| Kod | Anlam |
|-----|-------|
| 1001 | Geçersiz veya expired token |
| 1002 | Username zaten alınmış |
| 1003 | Yanlış username veya şifre |
| 1004 | Kaynak bulunamadı (media, user, comment) |
| 1005 | Yetkisiz işlem (başkasının medyası/yorumu) |
| 1006 | Geçersiz parametre (kısa şifre, geçersiz format vb.) |
| 1007 | Dosya boyutu limit aşımı |
| 1008 | Desteklenmeyen dosya formatı |
| 1009 | Private medyaya erişim engellendi |
| 2001 | Bozuk komut formatı |
| 2002 | Sunucu iç hatası |

## 3. Validation Kuralları

- **Username:** 3-20 karakter, sadece `[a-zA-Z0-9_]`. Boşluk/Türkçe karakter yok (parser kırılmasın).
- **Şifre:** Minimum 6 karakter. Server'da `SHA256(password + salt)` olarak saklanır.
- **Maksimum dosya boyutu:** Resim 20 MB, video 100 MB.
- **Desteklenen formatlar:** Resim: `jpg`, `jpeg`, `png`. Video: `mp4`, `mov`, `mkv`. Server uzantıdan kontrol eder.

---

## 4. Komutlar

### 4.1 REGISTER — Yeni hesap oluştur

```
İstek:  REGISTER <username> <password>\n
Cevap:  OK <token>\n
        ERR 1002 Username taken\n
        ERR 1006 Invalid username or password\n
```
Auth gerektirmez. Başarılı kayıttan sonra otomatik login olur, token döner.

### 4.2 LOGIN — Var olan hesaba giriş

```
İstek:  LOGIN <username> <password>\n
Cevap:  OK <token>\n
        ERR 1003 Wrong username or password\n
```

### 4.3 LOGOUT — Token'ı geçersiz kıl

```
İstek:  LOGOUT <token>\n
Cevap:  OK\n
```

### 4.4 UPLOAD — Medya yükle

```
İstek:    UPLOAD <token> <type> <visibility> <filename> <size>\n
          type: image | video
          visibility: public | private
          size: byte cinsinden
Cevap 1:  READY\n           (server hazır, payload bekliyor)
          ERR 1007 ...\n    (boyut limiti)
          ERR 1008 ...\n    (format desteklenmiyor)
Payload:  Client <size> byte raw data gönderir
Cevap 2:  OK <media_id>\n   (32 karakter UUID)
          ERR 2002 ...\n
```
- Payload **chunk chunk** gönderilmeli (örn. 64KB). Server `recv()` döngüsü ile toplam `size` byte alana kadar okur.
- Bağlantı upload sırasında koparsa server yarım dosyayı siler.

### 4.5 LIST — Feed listele

```
İstek:  LIST <token> <sort> <offset> <limit>\n
        sort: newest | most_downloaded | most_liked
        offset: int (skip count, sayfalama için)
        limit: int (max 20)
Cevap:  OK <count>\n<json>\n
        json: [{"id":"...","type":"image","visibility":"public","owner":"ali",
                "filename":"foo.jpg","size":12345,"created_at":1714400000,
                "download_count":12,"like_count":3,"liked_by_me":true}, ...]
```
- Private medyalar sadece sahibi LIST çekerse görünür. Diğer kullanıcıların feed'inde görünmez.
- `liked_by_me`: o anki kullanıcı bu medyayı beğenmiş mi?

### 4.6 DOWNLOAD — Medyayı tam boyutta indir

```
İstek:   DOWNLOAD <token> <media_id>\n
Cevap:   OK <type> <size>\n
Payload: Server <size> byte raw data gönderir
         ERR 1004 Not found\n
         ERR 1009 Private media\n
```
- Private medyayı sadece sahibi indirebilir.
- Başarılı her indirmede server `download_count++`.

### 4.7 PREVIEW — Resim için thumbnail indir

```
İstek:   PREVIEW <token> <media_id>\n
Cevap:   OK <size>\n
Payload: Server <size> byte JPEG thumbnail gönderir (max 256x256)
         ERR 1004 Not found\n
         ERR 1006 Preview not available  (video için)
```
- Server upload sırasında resim için thumbnail üretir ve diskte saklar.
- Video için preview yoktur (v1'de). Client video item'ları için kendi UI'ında play ikonu gösterir.

### 4.8 DELETE — Medya sil

```
İstek:  DELETE <token> <media_id>\n
Cevap:  OK\n
        ERR 1004 Not found\n
        ERR 1005 Not your media\n
```
- Sadece sahibi silebilir.
- Hard delete: dosya diskten, kayıt DB'den, ilgili likes ve comments cascade silinir.

### 4.9 LIKE / UNLIKE — Beğen / beğenmekten vazgeç

```
İstek:  LIKE <token> <media_id>\n
Cevap:  OK <new_like_count>\n
        ERR 1004 Not found\n

İstek:  UNLIKE <token> <media_id>\n
Cevap:  OK <new_like_count>\n
```
- Aynı kullanıcı aynı medyayı tekrar beğenirse server idempotent davranır (count artmaz).
- Private medya beğenilemez (görünmüyor zaten).

### 4.10 USER_MEDIA — Profil sayfası için kullanıcının medyaları

```
İstek:  USER_MEDIA <token> <username> <offset> <limit>\n
Cevap:  OK <count>\n<json>\n
```
- Format LIST ile aynı.
- Başkasının profilinde sadece public medyalar listelenir. Kendi profilinde public+private hepsi.

### 4.11 CHANGE_PASSWORD — Şifre değiştir

```
İstek:  CHANGE_PASSWORD <token> <old_password> <new_password>\n
Cevap:  OK <new_token>\n
        ERR 1003 Wrong password\n
        ERR 1006 New password too short\n
```
- Tüm eski tokenlar invalidate olur, yeni token döner.

---

## 5. Yorum Komutları (Stretch Goal — Gün 9 sonrası)

Diğer özellikler bitmezse atlanacak. Protokol yeri ayrılmıştır.

### 5.1 ADD_COMMENT
```
İstek:  ADD_COMMENT <token> <media_id> <text>\n
        text: boşluk içerebilir, satır sonuna kadar okunur
Cevap:  OK <comment_id>\n
```
> **Not:** Yorum komutu için parser özel davranır — `<media_id>` boşluğa kadar okunur, kalan her şey `\n`'e kadar `text` sayılır. Yani text içinde boşluk serbest, ama `\n` yasak (max 500 karakter).

### 5.2 LIST_COMMENTS
```
İstek:  LIST_COMMENTS <token> <media_id> <offset> <limit>\n
Cevap:  OK <count>\n<json>\n
        json: [{"id":"...","author":"ali","text":"güzel","created_at":1714...}, ...]
```

### 5.3 DELETE_COMMENT
```
İstek:  DELETE_COMMENT <token> <comment_id>\n
Cevap:  OK\n
        ERR 1005 Not your comment\n
```
- Yorum sahibi VEYA medyanın sahibi silebilir.

---

## 6. Örnek Akış

Yeni kullanıcı kayıt → resim yükle → feed'i göster → beğen → indir:

```
C: REGISTER inan superpass123\n
S: OK 7f3a9c2e1b8d4a5f6e9c0d1a2b3c4d5e\n

C: UPLOAD 7f3a9c2e... image public dogal.jpg 245678\n
S: READY\n
C: <245678 byte JPEG data>
S: OK ab12cd34ef56...\n

C: LIST 7f3a9c2e... newest 0 20\n
S: OK 1\n[{"id":"ab12cd34...","type":"image","owner":"inan",...}]\n

C: LIKE 7f3a9c2e... ab12cd34ef56...\n
S: OK 1\n

C: DOWNLOAD 7f3a9c2e... ab12cd34ef56...\n
S: OK image 245678\n
S: <245678 byte JPEG data>
```

---

## 7. Server Davranış Kuralları

- Aynı socket üzerinde birden fazla komut sırayla işlenir (pipelining).
- Her client kendi thread'inde çalışır. SQLite write işlemleri için global mutex.
- Geçersiz token gelirse her komut `ERR 1001` ile cevaplanır, REGISTER/LOGIN hariç.
- Bozuk format: `ERR 2001` döner ve **bağlantı kapatılır** (parser senkronizasyonu bozulmasın).
- Server log'u: her komut için `[timestamp] [client_ip] [username] <command> -> <result>` şeklinde stdout'a basılır.

---

## 8. Versiyon Notu

- **v1.0** (29 Nisan 2026): İlk sürüm. Yorum komutları stretch goal olarak işaretli.
- Protokol değişikliği = ikinizin onayı + version bump + commit message'da `[PROTOCOL]` etiketi.
