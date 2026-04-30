# Yapay Zeka ile Çalışma Rehberi — Pinterest-Clone-BIL-314

Selam! Bu projede kod yazarken Claude (veya ChatGPT, Cursor, vs.) gibi AI asistanları kullanacağız. Doğru kullanırsan **çok hızlı ilerlersin**. Yanlış kullanırsan saatler kaybedersin. Bu rehber sana doğru kullanmayı öğretecek.

---

## Önce Önemli Bir Şey

**Yapay zeka kod yazıyor diye sen kodu anlamak zorundasın.** Yoksa:
- Hocaya sunum yaparken sorduğunda donarsın
- Bug çıktığında ne olduğunu anlamazsın → çözemezsin
- Arkadaşın "bu kısmı niye böyle yaptın" dediğinde cevap veremezsin

**Kural:** AI yazsın, **sen oku ve anla**, sonra commit'le. Tek satırı bile anlamıyorsan dur, AI'a "bu satır ne yapıyor" diye sor.

---

## Hangi AI'ı Kullanmalısın?

İnan Claude kullanıyor (Anthropic'in modeli). Sen de aynısını kullanırsan **tutarlı** olur — projenin geçmişini, kararları, yaklaşımı bilen tek bir asistan olur.

**Önerim: [claude.ai](https://claude.ai)**

- Ücretsiz hesap aç
- Web tarayıcıdan kullanabilirsin
- Mobile app'i de var

Alternatif olarak **ChatGPT** (chatgpt.com) da iyi, ama bu projede Claude'un kararlarını takip ediyoruz, ChatGPT bazen farklı yöne çekebilir.

---

## ALTIN KURAL: CONTEXT VER

AI'ın tek bir mesajını gördüğünde **proje hakkında hiçbir şey bilmez**. Eğer şöyle yazarsan:

> "C++ ile login fonksiyonu yaz"

Sana **rastgele** bir login fonksiyonu yazar. Ama bizim projeye uymayabilir:
- Yanlış kütüphane kullanır
- Bizim protokolümüze uymaz
- Bizim DB şemamızı bilmez

**Doğru yaklaşım:** Her yeni sohbette önce **proje context'ini** ver:

```
Selam, BİL314 Pinterest-Clone projesinde [SERVER/CLIENT] tarafını yapıyorum.
Aşağıdaki dosyaları yükleyip projeye hızlıca aşina olur musun?

[Dosyaları ekle]
```

Yüklemen gereken dosyalar:
1. **`docs/CLAUDE_CONTEXT.md`** — projenin durumu, kararlar, stack
2. **`docs/PROTOCOL.md`** — server-client haberleşme protokolü
3. **`docs/schema.sql`** — veritabanı şeması
4. **`docs/ROADMAP.md`** — gün gün plan (varsa)

Bu dört dosya AI'ın ne yapacağını bilmesi için yeter. Her yeni sohbette tekrar yükle — AI'ın hafızası sohbetler arası taşınmaz.

---

## Ne Zaman AI Sohbeti Açmalısın?

### ✅ Yeni sohbet aç (clean slate)
- Yeni bir özelliğe başlıyorsan ("login formu yapacağım")
- Önceki sohbet 50+ mesaja ulaştıysa (uzayınca AI yavaşlar/karışır)
- Tamamen farklı bir konuya geçtiyseniz (auth bitti, upload başlıyorsun)

### ❌ Yeni sohbet açma
- Aynı bug üstünde çalışıyorsan (context kayar)
- Henüz ortada bir kod yokken (önce planı bitir)

---

## Doğru Prompt Nasıl Yazılır?

**Kötü:** "Login fonksiyonu yaz"
**İyi:** "PROTOCOL.md'deki 4.2 LOGIN komutunu işleyen bir handler yaz. Username ve password parametre olarak gelecek, SQLite'tan SHA256 hash'i kontrol edip doğruysa yeni token üretip dön. Hata durumlarını PROTOCOL.md hata kodlarına göre döndür."

Fark:
- Kötü prompt: AI rastgele yapar
- İyi prompt: AI tam istediğini yapar

### İyi Prompt'un 4 Bileşeni

1. **Bağlam:** "Projemizin X kısmındayım"
2. **Hedef:** "Şunu yapmak istiyorum"
3. **Kısıtlar:** "Bu kütüphaneyi kullan, bu standartı takip et"
4. **Çıktı formatı:** "Tam dosya kodu ver / sadece eklenecek satırları ver / açıklama ile birlikte ver"

### Örnek Prompt'lar

**Yeni özellik için:**
```
PROTOCOL.md'deki 4.4 UPLOAD komutunu server tarafında implement etmek istiyorum.

Şu anda main.cpp'de basit bir TCP echo server var. Modüler bir yapı için:
- protocol.hpp/cpp dosyalarımız var (komut parser)
- session.hpp/cpp var (per-client thread)
- db.hpp/cpp var (SQLite wrapper)
- media.hpp/cpp var (medya işlemleri)

UPLOAD komutu media.hpp/cpp içinde olmalı. Chunk'lı dosya transferi yapacak,
maks resim 20MB / video 100MB. Önce kodu yaz, sonra her satırı kısa açıkla.
```

**Bug için:**
```
LOGIN handler'ım bazen "ERR 1003" dönüyor doğru şifrede bile.
İşte kod:
[kodu yapıştır]

İşte test çıktısı:
[terminal çıktısını yapıştır]

Soruna nereden bakmaya başlamam lazım?
```

**Anlamak için:**
```
Bu kod blokunu satır satır açıklar mısın, özellikle "&" ve "::" ne işe yarıyor?
[kodu yapıştır]
```

---

## AI'ın Verdiği Kodu Nasıl Kullanmalısın?

### Adım 1: Önce OKU, anlama

AI sana 50 satır kod verdi. **Direkt commit'leme!** Önce:
- Her satırı oku
- Anlamadığın yer varsa AI'a sor: "şu satır ne yapıyor"
- Mantık olarak doğru mu, kafanda canlandır

### Adım 2: Kopyala, kendi dosyana yapıştır

Doğrudan repo'daki dosyalara yapıştır:
- VSCode aç (WSL bağlantısıyla)
- İlgili dosyayı aç
- Kodu yapıştır

### Adım 3: Derlemeye dene

```bash
cd ~/Pinterest-Clone-BIL-314/server/build
cmake --build . -j$(nproc)
```

Hata varsa:
- Hatayı **olduğu gibi** AI'a yapıştır
- "Şu hatayı alıyorum, sebep ne, nasıl düzeltirim" diye sor

### Adım 4: Test et

Çalıştırdığında beklediğin gibi davranıyor mu?
- Localhost'ta `nc localhost 8080` ile manuel test
- Birkaç edge case dene (boş input, çok uzun input)

### Adım 5: Anladığından emin olunca commit et

```bash
git add .
git commit -m "Server: UPLOAD handler implementasyonu"
git push
```

---

## AI'ı Nasıl YANLIŞ Kullanır İnsanlar?

❌ **"Tüm projeyi yaz" demek** — AI 200 satırlık çöp kod verir, hiçbiri çalışmaz
❌ **Anlamadan kopyalamak** — sunumda donarsın, debug edemezsin
❌ **Her küçük şey için yeni sohbet** — context kaybolur, AI tekrar tekrar açıklamak zorunda kalırsın
❌ **Hata mesajını yapıştırmadan "çalışmıyor" demek** — AI tahmin yapar, çoğunlukla yanlış
❌ **AI'ın ilk cevabını kabul etmek** — bazen daha iyi yol vardır, "alternatif var mı" diye sor
❌ **AI ile arkadaşa karşı tartışmak** — AI sana her ikiniz için "haklısın" der, gerçek karar size kalmış

---

## İyi Pratikler

### 🟢 Kısa, net mesajlar yaz

Uzun bir mesaj yazma. **Birden fazla soruyu** birden sorma. Tek konu, tek soru, tek cevap.

### 🟢 Hata mesajını **TAM olarak** yapıştır

```
hatasını alıyorum
```
yerine:
```
İşte tam hata mesajı:

server/src/auth.cpp: In function 'pc::LoginResult pc::login_user(...)':
server/src/auth.cpp:42:18: error: 'sqlite3_prepare_v2' was not declared in this scope
   42 |             if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
      |                  ^~~~~~~~~~~~~~~~~~~

Hangi include'u eksik bırakmışım?
```

### 🟢 Kodu **markdown code block** içinde yapıştır

```cpp
void foo() {
    // ...
}
```

Bu şekilde formatlanırsa AI daha iyi okur. Üç tilde + dil adı.

### 🟢 Birden fazla dosya gerekiyorsa hepsini ver

"Login fonksiyonum DB'ye erişiyor, hata alıyorum" derken hem `auth.cpp`'yi hem `db.cpp`'yi yapıştır.

### 🟢 "Niye böyle yaptın" diye sor

AI bir karar verdi, sen alternatifi merak ettin. Sor:
> "Niye `std::map` yerine `std::unordered_map` kullandın? Hangisi daha iyi?"

Bu sayede sen de öğrenirsin.

### 🟢 Sunum öncesi "neyi nasıl açıklayım" diye sor

Sunum yaklaşırken AI'a sor:
> "Hocaya UPLOAD handler'ımı 1 dakikada nasıl anlatırım? Kritik noktalar neler?"

AI sana özet çıkarır, sunum kalitesi artar.

---

## Sık Çıkacak Senaryolar ve Prompt Önerileri

### Senaryo 1: Yeni dosya yazacaksın

```
[CONTEXT_DOSYALARI ekle]

Şimdi server/src/auth.cpp dosyasını yazmak istiyorum. Bu dosya:
- auth.hpp'de tanımlı fonksiyonları implement edecek
- db.hpp'deki Database sınıfını kullanacak
- util.hpp'deki sha256_hex ve random_hex'i kullanacak
- PROTOCOL.md'deki REGISTER ve LOGIN davranışını sağlayacak

İşte mevcut dosyalar:
[auth.hpp içeriğini yapıştır]
[db.hpp içeriğini yapıştır]
[util.hpp içeriğini yapıştır]

Tam auth.cpp dosyasını yaz, her fonksiyonun üstüne kısa yorum koy.
```

### Senaryo 2: Mevcut kodu değiştireceksin

```
Şu anda LOGIN handler'ım çalışıyor ama sadece tek thread güvenli değil.
İşte mevcut kod:

[auth.cpp'nin ilgili kısmını yapıştır]

İki istemci aynı anda LOGIN gönderirse SQLite'ta race condition oluşabilir.
Mutex ekleyerek thread-safe yapmak istiyorum. Sadece değişen satırları
göster, tüm dosyayı tekrar yazma.
```

### Senaryo 3: Bug çözüyorsun

```
UPLOAD komutunda dosya yarım kalıyor. İşte test akışı:

1. Client: "UPLOAD <token> image public test.jpg 245678\n" gönderiyor
2. Server: "READY\n" cevap veriyor
3. Client: 245678 byte JPEG gönderiyor
4. Server: 100KB sonra "OK <media_id>\n" dönüyor ama dosya 100KB

Server tarafında recv döngüsü:
[ilgili kodu yapıştır]

Server logu:
[log çıktısını yapıştır]

Ne yanlış yapıyor olabilirim?
```

### Senaryo 4: Hocaya sunum hazırlığı

```
[CONTEXT_DOSYALARI ekle]

Yarın hocaya 10 dakikalık sunum yapacağım. Bizim CLIENT tarafımızı anlatacağım.
Şu konular yer alacak:
- Qt6 kullanma sebebi
- QTcpSocket ile asenkron iletişim
- Pinterest grid layout

Her konu için 30 saniyelik konuşma metni hazırlar mısın? Öğrenci dilinde,
çok teknik olmasın ama özet bilgi versin.
```

---

## Claude'a Özel İpuçları

İnan Claude kullanıyor, sana da Claude öneriyorum. Birkaç özel taktik:

### 1. "Düşünmesini" iste

```
Bu sorunu çözmeden önce sesli düşün, hangi yaklaşımları
değerlendirebileceğimizi listele.
```

Claude bu durumda direkt kod yazmaz, önce mantığı tartar. Daha kaliteli sonuç çıkar.

### 2. "Eleştir" iste

Bir tasarımın iyi mi kötü mü emin değilsen:
```
Şu mimariyi kuracağım: [X yaklaşımı]. Bunun zayıf yanları neler?
Daha iyi bir alternatifi var mı?
```

Claude direkt onaylamaz, gerçekten düşünür.

### 3. Adım adım iste

```
Şu özelliği eklerken adım adım gidelim. Sen bana plan ver, ben her adımı
yapıp test edip dönüyim. Tek seferde 200 satır kod isteme.
```

Bu sayede her küçük parça çalıştığını test edersin, bug çıkarsa nerede çıktığı belli.

### 4. "Türkçe açıkla" diye belirt

Claude varsayılan olarak Türkçe yazar (sen Türkçe yazdığında) ama bazen kod yorumlarında İngilizce kalır. Açıkça söylersen tutarlı olur:
```
Lütfen tüm açıklamaları ve kod yorumlarını Türkçe yaz.
```

---

## Pratik Tavsiyeler

### Sohbet düzenli kalsın

Her büyük özellik için ayrı sohbet aç ve **adlandır** (Claude.ai'da sol kenarda sohbet listesi var, sağ tık → Rename):
- "Auth sistemi (REGISTER/LOGIN)"
- "UPLOAD handler"
- "Client login UI"
- "Bug: UPLOAD chunk problem"

İleride "şu konuyu nerede konuşmuştuk" diye ararsın, kolay bulursun.

### Önemli kararları not al

AI ile bir karar verdin (örn "thread başına SQLite connection açacağız"). Bu kararı:
- `docs/DECISIONS.md` dosyasına yaz
- Veya commit mesajına yaz
- Veya İnan'a Discord'da paylaş

İleride "neden böyle yapmıştık" diye merak ederiz, kayıt olsun.

### İnan ile aynı sayfada ol

İkiniz farklı AI sohbetleri kullanıyorsanız ve aynı kod parçasına dokunuyorsanız, **birbirinizle konuşmadan büyük değişiklik yapmayın**. AI her ikinize de farklı önerebilir, çakışırsınız.

---

## "Yardım Et" Demeden Önce Kontrol Listesi

Bir bug'la karşılaştın, "İnan'a sorayım" diye düşündün. Önce:

- [ ] Hata mesajını okudum, anladım mı?
- [ ] AI'a hata mesajını yapıştırıp sordum mu?
- [ ] AI'ın önerisini denedim mi?
- [ ] Stack Overflow'a baktım mı? (hata mesajını Google'a yapıştır)
- [ ] Son 5 dakikada ne değiştirdim, geri alabilir miyim? (`git diff`)

Bunlardan hiçbiri çözmediyse İnan'a yaz. **Yapacaklar:**
1. Hangi adımda olduğun (örn "UPLOAD handler yazıyorum")
2. Hangi komutu/aksiyonu denedin
3. Tam hata mesajı (ekran görüntüsü veya text)
4. AI'ın ne dediği (varsa)

---

## Son Uyarı: AI Bazen YANLIŞ Söyler

AI mükemmel değil. Bazen:
- Var olmayan kütüphane fonksiyonu önerir
- Eski API kullanır (artık çalışmıyor olabilir)
- Mantık hatası yapar
- "Bu doğru" der ama derlenmez

**Her AI cevabını şüpheyle yaklaş.** Çalıştırmadan önce:
1. Mantık olarak doğru mu, kafanda canlandır
2. Derle/çalıştır → gerçek dünyada test et
3. Hata varsa AI'a geri dön, "şu hata, ne oldu" diye sor

İlk cevap %100 doğru olmayabilir, **iterasyon** lazım.

---

## Özet — Bunu Hatırla

```
1. Yeni sohbet aç → context dosyalarını yükle
2. Net, kısa prompt yaz
3. AI cevap versin → OKU, ANLA
4. Kodu kopyala → repo'na yapıştır → derle → test et
5. Çalışırsa commit'le; çalışmazsa hatayı AI'a yapıştır, devam
6. Anlamadığın yeri sor, AI sana açıklasın
7. Önemli kararları not al, İnan'la paylaş
```

İyi şanslar! Yapay zeka iyi bir takım arkadaşı, ama **sen şofşörsün, o navigasyon**. Yön sen veriyorsun. 🚀

---

## Hızlı Başlangıç Checklist (İlk Sohbetin İçin)

```
☐ claude.ai veya chatgpt.com aç, hesap aç
☐ İnan'dan şu dosyaları al:
    - docs/CLAUDE_CONTEXT.md
    - docs/PROTOCOL.md
    - docs/schema.sql
    - docs/ROADMAP.md (varsa)
    - docs/GIT_REHBERI.md (varsa)
☐ Yeni sohbet aç
☐ İlk mesaj:
    "Selam, BİL314 Pinterest-Clone projesinde [SERVER veya CLIENT] tarafını
     yapıyorum. Önce projeye aşina olman için aşağıdaki dosyaları
     yüklüyorum, oku ve hazır olduğunu söyle. Sonra ilk işime başlayalım."
☐ Dosyaları sürükle-bırak yükle
☐ AI hazır olduğunu söyleyince ilk işine başla
```
