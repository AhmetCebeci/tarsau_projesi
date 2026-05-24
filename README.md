# 📦 Tarsau - Arşivleme Programı

Linux üzerinde C diliyle geliştirilmiş, metin dosyalarını (.txt ve .dat) tek paket haline getiren ve orijinal izinleriyle geri çıkaran bir arşivleme yazılımıdır.

---

## Özellikler

- Giriş Kontrolü: Sadece .txt ve .dat uzantılı metin dosyalarını işler.
- Kapasite: En fazla 32 dosya ve maksimum 200 MB toplam boyut destekler.
- Çıktı Yönetimi: -o kullanılmazsa varsayılan olarak a.sau adını alır.
- Dizin Otomasyonu: Hedef klasör yoksa otomatik oluşturulur.
- İzin Koruma: Dosyaların Linux okuma, yazma, çalıştırma (rwx) izinlerini aynen geri yükler.
- Kararlılık: Hatalı parametre veya bozuk arşiv durumunda çökmeden hata mesajı verir.

---

## Derleme ve Çalıştırma

Derlemek için:
make

Kalıntıları temizlemek için:
make clean

---

## Kullanım Senaryoları

### 1. Paketleme Modu (-b)
Dosyaları tek bir .sau arşivinde toplar.

İsim belirterek arşivleme:
./tarsau -b dosya1.txt dosya2.dat -o odevim.sau

Varsayılan isimle (a.sau) arşivleme:
./tarsau -b dosya1.txt dosya2.txt

Hatalı format testi:
./tarsau -b dosya1.txt t7.mp3
Çıktı: t7.mp3 giris dosyasinin formati uyumsuzdur!

### 2. Arşivden Çıkarma Modu (-a)
Arşiv içeriğini dışarı çıkarır.

Belirli klasöre çıkarma:
./tarsau -a odevim.sau hedef_klasor

Bulunulan dizine çıkarma:
./tarsau -a odevim.sau

Fazla parametre testi:
./tarsau -a odevim.sau klasor1 klasor2
Çıktı: Arsiv dosyasi uygunsuz veya bozuk!

Hatalı uzantı testi:
./tarsau -a dosya1.txt
Çıktı: Arsiv dosyasi uygunsuz veya bozuk!

---

## Arşiv Yapısı

1. Organizasyon Bölümü: İlk 10 bayt, bu bölümün toplam karakter uzunluğunu tutar (Örn: 0000000125). Ardından her dosya verisi |dosya_adi,izin,boyut| şeklinde eklenir.
2. Veri Bölümü: Meta-veri alanı bittiği andan itibaren tüm kaynak dosyaların içerikleri peş peşe arşive yazılır.
