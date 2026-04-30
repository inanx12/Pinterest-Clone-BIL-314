# Git & GitHub Rehberi — Pinterest-Clone-BIL-314

Selam! Bu rehber projeye katılırken bilmen gereken Git komutlarını sıfırdan anlatıyor. Daha önce komut satırından Git kullanmadıysan bile takip edebilirsin. Sırayla oku, atlama.

---

## Git Nedir, Niye Kullanıyoruz?

İki kişi aynı projede çalışacağız. Eğer dosyaları WhatsApp'tan/email'den paylaşırsak:
- Kim hangi dosyanın son halini bilmiyor
- Aynı dosyayı ikimiz de değiştirdik mi karışıyor
- Eski hale dönmek imkansız

**Git** bu sorunları çözüyor. **GitHub** ise Git'in bulutta saklandığı yer. Kod GitHub'da duruyor, ikimiz de oradan çekip oraya gönderiyoruz.

---

## Üç Önemli Yer

```
[Senin makinende dosyalar]  →  [Senin Git history'n]  →  [GitHub]

       Working                       Local                Remote
       Directory                      Repo                  Repo
```

- **Working Directory:** Düzenlediğin dosyalar (örnek: `main.cpp` üstünde çalışıyorsun)
- **Local Repo:** Senin makinendeki Git history (commit'ler burada)
- **Remote (GitHub):** Bulutta, paylaştığımız yer

Komutlar bu üç yer arasında dosya hareket ettirir.

---

## Temel 6 Komut

Bu altısı %95'ini halleder:

| Komut | Ne yapar |
|-------|----------|
| `git status` | Şu an ne durumdayım? |
| `git pull` | GitHub'dan son hali çek |
| `git add .` | Değişikliklerimi "commit'e hazır" yap |
| `git commit -m "mesaj"` | Bu değişiklikleri kaydet |
| `git push` | Kaydettiklerimi GitHub'a gönder |
| `git log --oneline` | Geçmiş commit'leri gör |

---

## Komutları Tek Tek Açıklayalım

### 1. `git status` — En Çok Kullanacağın Komut

Her şeyden önce, her komuttan önce bunu çalıştır. Sana **şu an ne durumda olduğunu** söyler:

```bash
git status
```

Olası çıktılar:

**A) Hiçbir şey değişmemiş, her şey güncel:**
```
On branch main
Your branch is up to date with 'origin/main'.
nothing to commit, working tree clean
```
→ Yapacak iş yok, devam et.

**B) Dosya değiştirdin ama henüz işlemedin:**
```
Changes not staged for commit:
  modified:   README.md
```
→ `git add` yapman gerek.

**C) Add ettin ama commit etmedin:**
```
Changes to be committed:
  modified:   README.md
```
→ `git commit` yapman gerek.

**D) Commit ettin ama GitHub'a göndermedin:**
```
Your branch is ahead of 'origin/main' by 1 commit.
```
→ `git push` yapman gerek.

**Kural:** Şüphede kaldığında `git status` çalıştır. Sana ne yapman gerektiğini söyler.

---

### 2. `git pull` — GitHub'dan Son Hali Çek

Çalışmaya başlamadan **MUTLAKA** bunu çalıştır. İnan'ın gönderdiği son değişiklikleri sana getirir:

```bash
git pull
```

Çıktı genelde:
```
Already up to date.
```
veya
```
Updating xxxxx..yyyyy
Fast-forward
 docs/ROADMAP.md | 50 ++++++++++++++
 1 file changed, 50 insertions(+)
```

İkincisi "yeni dosyalar geldi" demek. ✅

**Kritik Kural:** Hiçbir komut yazmadan önce, kod yazmaya başlamadan önce **her oturum** `git pull` ile başla. Aksi halde push ederken hata alırsın.

---

### 3. `git add` — Değişikliği "Commit'e Hazır" Yap

Bir dosyayı değiştirdin (örn `README.md`). Git bunu otomatik commit etmez, **önce işaretlemen** lazım:

```bash
# Sadece bir dosya:
git add README.md

# Birden fazla dosya:
git add README.md docs/notes.md

# Tüm değişiklikleri (en pratik):
git add .
```

`.` o anki klasör ve **tüm alt klasörlerdeki** değişiklikleri ekler.

> **Güvenlik notu:** `.gitignore` dosyasında listelenen şeyler (örn `build/` klasörü) `git add .` yapsan bile eklenmez. Bu güvenli, build dosyalarını yanlışlıkla göndermezsin.

---

### 4. `git commit` — "Şu Değişikliği Kaydediyorum" De

```bash
git commit -m "Server: REGISTER handler eklendi"
```

`-m` parametresi mesaj demek. **Mesaj çok önemli**, ileride "ne yaptım" diye anlaman için:

✅ **İyi mesajlar:**
- `Server: REGISTER handler implementasyonu`
- `Client: login formu eklendi`
- `Fix: UPLOAD chunk size hatası giderildi`
- `Docs: README'ye build talimatları eklendi`

❌ **Kötü mesajlar:**
- `update`
- `fix`
- `son hali`
- `asdfg`

Format: `[Modül]: kısa açıklama` veya `[Modül]: ne değişti` şeklinde olsun. Türkçe veya İngilizce fark etmez, tutarlı olalım yeter.

---

### 5. `git push` — GitHub'a Gönder

```bash
git push
```

Bu kadar. Kaydettiklerin GitHub'a gider, ben (İnan) `git pull` yapınca senin değişikliklerini görürüm.

> **İlk push'ta farklı:** Yeni bir branch açtıysan ilk push'ta şöyle yazarsın:
> ```bash
> git push -u origin <branch_adi>
> ```
> `-u origin <ad>` "bu lokal branch GitHub'taki ile bağlı olsun" demek. Sadece **ilk seferde** lazım.

---

### 6. `git log --oneline` — Geçmişi Gör

Şimdiye kadar yapılan commit'leri kısa formatta listeler:

```bash
git log --oneline
```

Çıktı:
```
af11709 Add 11-day project roadmap
b2c3d4e Update README with build instructions
1a2b3c4 Server: TCP echo skeleton (v0.1)
...
```

Hangi commit ne yaptı, kim yaptı bilmek için süper.

`q` tuşuna basarak çıkarsın.

---

## Tipik Günlük Akış (Bunu Ezberle)

Her oturum şöyle başlar ve biter:

```bash
# 1. Repo klasörüne gel
cd ~/Pinterest-Clone-BIL-314

# 2. Son hali çek (önce!)
git pull

# 3. Şimdi nerede olduğunu anla (opsiyonel ama öğretici)
git status

# ... kod yaz, derle, test et ...

# 4. Ne değişti gör
git status

# 5. Hepsini stage'le
git add .

# 6. Anlamlı mesajla kaydet
git commit -m "Server: REGISTER handler implementasyonu"

# 7. GitHub'a gönder
git push
```

İlk üç komut: `cd`, `pull`, `status`. Sonraki üç komut: `add`, `commit`, `push`. Toplam 30 saniye.

---

## Hata Durumları ve Çözümleri

### "Push reddedildi" / `! [rejected]` hatası

Sebep: GitHub'da senin lokalinde olmayan yeni commit var. İnan push etmiş, sen pull etmemişsin.

```
! [rejected]        main -> main (fetch first)
error: failed to push some refs to 'github.com:...'
```

Çözüm:
```bash
git pull --rebase
git push
```

`--rebase` senin commit'ini İnan'ın commit'inin üstüne yerleştirir, history düz kalır.

### "Merge conflict" / Çakışma

İnan ile sen aynı dosyada aynı satırı değiştirmişsiniz. Git "ben karar veremiyorum" diyor, sana sokuyor.

```
CONFLICT (content): Merge conflict in <dosya_adi>
```

Çözüm:
1. Hangi dosya çakıştı gör:
   ```bash
   git status
   ```

2. Dosyayı `nano <dosya_adi>` ile aç. İçinde şuna benzer satırlar var:
   ```
   <<<<<<< HEAD
   benim kodum
   =======
   inan'ın kodu
   >>>>>>> commit-hash
   ```

3. **İstediğini bırak**, işaretleri (`<<<<<<<`, `=======`, `>>>>>>>`) sil. İkisini birleştirebilir veya birini seçebilirsin.

4. Kaydet (`Ctrl+O → Enter → Ctrl+X`)

5. Devam et:
   ```bash
   git add <dosya_adi>
   git rebase --continue   # eğer pull --rebase ile geldiyse
   # veya
   git commit              # eğer pull --merge ile geldiyse
   git push
   ```

> **Çakışmadan kaçınmanın yolu:** Aynı dosyaya ikimiz de dokunmamak. İnan `server/` klasöründe çalışacak, sen `client/` (veya hangi tarafı seçtinse). Ortak dosyalar (PROTOCOL.md, schema.sql, README.md) için **önce konuşalım**, sonra düzenleyelim.

### "Yanlış dosyayı stage'ledim, geri al"

```bash
git restore --staged <dosya_adi>
```

Stage'den çıkarır, dosya değişikliğin durur, kaybolmaz.

### "Commit mesajını yanlış yazdım"

**Sadece henüz push etmediysen:**
```bash
git commit --amend -m "yeni doğru mesaj"
```

Push ettiysen dokunma, böyle kalsın.

### "Ah, kötü kod yazdım, bu commit'i geri al ama kodumu kaybetme"

```bash
git reset --soft HEAD~1
```

Son commit silinir, değişiklikler staging'de kalır. Düzeltir, yeniden commit edersin.

### "Bilmediğim bir şey oldu, panikteyim"

İlk yapacağın şey:
```bash
git status
git log --oneline
```

Sonra İnan'a (veya bana) **terminal çıktısını yapıştır**, sor. Rastgele komut çalıştırma, daha çok karıştırırsın.

---

## ASLA YAPMA Listesi

❌ **`git push --force` (ya da `-f`)** — uzaktaki commit'leri ezer, İnan'ın kodu kaybolur. Bu komutu sadece İnan veya Claude söylerse yap.

❌ **`.git` klasörünü silme** — Git'in tüm history'si orada, gider.

❌ **Bilmediğin git komutlarını rastgele çalıştırma** — Stack Overflow'dan kopyala-yapıştır en büyük tehlike. Şüphedeysen sor.

❌ **Şifre / API key / token commit etme** — Bir kere push ettin mi history'de kalır. Hassas dosyaları `.gitignore`'a ekle.

❌ **Aynı dosyaya İnan'la aynı anda dokunma** — Önce konuşun, sonra düzenleyin.

---

## Branch'ler (Sonradan Lazım Olacak)

İlk başta sadece `main` branch'ında çalışacaksın. Ama büyük özellikler için ayrı branch açacağız. Buna **feature branch** denir.

### Yeni branch aç ve geç

```bash
git checkout -b feature/client-login
```

Bu komut hem branch oluşturur hem oraya geçer.

### Branch'lar arasında geçiş

```bash
git checkout main                  # main'e dön
git checkout feature/client-login  # feature branch'a dön
```

### Hangi branch'tayım

```bash
git branch
```

`*` işaretli olan şu an aktif olan.

### Branch'ı GitHub'a ilk push

```bash
git push -u origin feature/client-login
```

### Pull Request (PR) Akışı

1. Feature branch'ında çalış, commit'leri at
2. Push et: `git push`
3. GitHub'da repo sayfasını aç
4. "Compare & pull request" yeşil butonu tıkla
5. Title yaz, açıklama yaz, "Create pull request"
6. İnan kodunu inceler, "Approve" verir
7. "Merge pull request" → main'e birleşir
8. Sen lokal'de:
   ```bash
   git checkout main
   git pull
   git branch -d feature/client-login   # eski branch'ı sil
   ```

> **Not:** Bu projede şimdilik PR zorunlu değil, ama kullanmaya alışmamızda fayda var.

---

## VSCode'da Git (Görsel Alternatif)

Komut satırı zorlanırsan VSCode'da yapabilirsin:

1. Sol kenarda dal ikonu (Source Control sekmesi) → tıkla
2. Değişen dosyalar listede çıkar
3. `+` → `git add` (stage)
4. Mesaj yaz, `✓` (commit)
5. `...` menüsü → Push

**Ama önce komut satırını öğren**, GUI ezberlemeyi geciktirir. Bu hafta hep terminalde yap, ileride GUI'ye geçebilirsin.

---

## Pratik Egzersiz (Bunu Yap, Pekişir)

### Egzersiz 1: README'ye satır ekle

```bash
cd ~/Pinterest-Clone-BIL-314
git pull
git status

# README'yi aç, sonuna kendi adını ekle
nano README.md
# (sonuna "Geliştirici: Senin Adın" diye satır ekle, Ctrl+O Enter Ctrl+X)

git status                          # değişiklik gözükür
git add README.md
git status                          # staged'e geçti
git commit -m "Add my name to README"
git status                          # ahead of origin/main by 1 commit
git push
git status                          # her şey güncel
```

GitHub'da repo sayfasını yenile, README değişti mi gör. ✅

### Egzersiz 2: Commit history'sine bak

```bash
git log --oneline
```

Son birkaç commit listelenir, kendi yaptığını göreceksin.

---

## Hızlı Hatırlatma Kartı

```
HER OTURUMUN BAŞINDA:
   git pull

KOD YAZARKEN:
   git status   (sık sık)

BİTİRDİĞİNDE:
   git add .
   git commit -m "anlamlı mesaj"
   git push

HATA ALDIYSAN:
   1. git status (önce ne durumda olduğunu gör)
   2. terminal çıktısını İnan'a/Claude'a yapıştır
   3. tahmin yapma, sor
```

---

## SSS

**S: Aynı anda hem ben hem İnan kod yazsak ne olur?**

C: Farklı dosyalardaysanız sorun yok, ikiniz de push edip pull ettikçe değişiklikler birleşir. Aynı dosyadaysanız çakışma olur, yukarıdaki "Merge conflict" bölümünü oku.

**S: Yanlışlıkla yanlış dosyayı sildim, geri alabilir miyim?**

C: Eğer henüz commit'lemedin: `git restore <dosya_adi>` yeterli. Commit'lediysen: `git checkout HEAD~1 -- <dosya_adi>` ile bir önceki halinden çekersin.

**S: Push'tan sonra "ah, sildiğim dosya lazımmış" diyebilir miyim?**

C: Evet. `git log --oneline` ile commit hash'ini bul, sonra `git show <hash>:<dosya_adi> > <dosya_adi>` ile geri getir.

**S: Türkçe karakterler problem yapar mı?**

C: Commit mesajlarında genelde sorun yok. Dosya adlarında **kullanma** (Türkçe dosya adı = path bug'ları). Kod içinde Türkçe string serbest, kaynak dosyalar UTF-8 olduğu sürece.

**S: Şifremi/API key'i yanlışlıkla commit ettim, ne yaparım?**

C: 🚨 Hemen İnan'a haber ver. Push ettiysen GitHub history'sinde kalır, key'i hemen iptal etmek lazım. Sadece commit ettiysen `git reset --soft HEAD~1` ile geri al, dosyayı düzelt, yeniden commit et.

---

## Yardım Lazımsa

- Hata mesajının **tam metni** veya **ekran görüntüsü** + **hangi komutu çalıştırdığın**
- Bu üçü olmadan sorun çözmek zor

İyi şanslar! İlk birkaç gün biraz garip gelir, bir hafta sonra parmakların ezbere yapar. 👍
