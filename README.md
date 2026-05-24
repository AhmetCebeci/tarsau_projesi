# Tarsau - Sıkıştırmasız Arşivleme Programı

Linux işletim sistemi üzerinde C dili kullanılarak geliştirilmiş, birden fazla metin dosyasını (.txt ve .dat) tek bir paket halinde birleştiren ve bu paketten dosyaları orijinal izinleriyle geri çıkaran bir arşivleme yardımcı yazılımıdır.

Bu çalışma, Sakarya Üniversitesi Bilgisayar Mühendisliği Bölümü Sistem Programlama dersi dönem projesi kapsamında geliştirilmiştir.

## Özellikler

- Giriş Dosyası Kontrolü: Sadece ASCII formatındaki .txt ve .dat uzantılı metin dosyalarını işler. Format dışı dosyalarda güvenli hata mesajı verir.
- Kapasite ve Sınırlar: Tek seferde en fazla 32 adet dosya paketleyebilir ve toplam girdi boyutu maksimum 200 MB olabilir.
- Dinamik Çıktı Yönetimi: -o parametresi kullanılmadığında varsayılan olarak a.sau adıyla arşiv oluşturur.
- Dizin Otomasyonu: Arşiv açılırken belirtilen hedef klasör sistemde yoksa otomatik olarak oluşturur.
- İzin Koruma: Paketlenen dosyalar arşivden çıkarılırken, ilk paketlendikleri andaki Linux okuma, yazma, çalıştırma (rwx) izin bitlerini tam olarak korur ve geri yükler.
- Kararlılık: Hatalı veya eksik parametre kullanımlarında, bozuk veya uygunsuz arşiv dosyalarında program aniden çökmez; anlamlı bir hata mesajıyla temiz bir şekilde sonlanır.

## Derleme

Proje içerisinde yer alan Makefile sayesinde derleme işlemi tek komutla gerçekleştirilebilir.

Derlemek için terminale yazılacak komut:
make

Bu komut sonucunda tarsau adında çalıştırılabilir bir binary dosya oluşturulur. 

Derlenen dosyaları ve kalıntıları temizlemek için terminale yazılacak komut:
make clean

## Kullanım ve Test Senaryoları

Program komut satırından alınan parametrelere göre iki farklı modda çalışmaktadır:

### 1. Dosyaları Birleştirme ve Paketleme Modu (-b)

Birden fazla metin dosyasını tek bir .sau arşivinde toplar.

Belirtilen isimde arşiv oluşturma komutu:
./tarsau -b dosya1.txt dosya2.dat -o odevim.sau

İsim belirtilmezse varsayılan olarak a.sau oluşturma komutu:
./tarsau -b dosya1.txt dosya2.txt

Hatalı format test senaryosu (Metin olmayan dosya girildiğinde):
./tarsau -b dosya1.txt t7.mp3
Çıktı: t7.mp3 giris dosyasinin formati uyumsuzdur!

### 2. Arşivden Çıkarma Modu (-a)

.sau uzantılı bir arşiv dosyasının içeriğini dışarı çıkarır.

Dosyaları belirli bir dizinin veya klasörün içine çıkarma komutu (Klasör yoksa otomatik açılır):
./tarsau -a odevim.sau hedef_klasor

Klasör belirtilmezse dosyaları doğrudan bulunulan dizine çıkarma komutu:
./tarsau -a odevim.sau

Fazla parametre girildiğinde engelleme senaryosu komutu:
./tarsau -a odevim.sau klasor1 klasor2
Çıktı: Arsiv dosyasi uygunsuz veya bozuk!

Uygun olmayan uzantı veya sahte dosya girildiğinde engelleme senaryosu komutu:
./tarsau -a dosya1.txt
Çıktı: Arsiv dosyasi uygunsuz veya bozuk!

## Arşiv Yapısı (.sau Formatı)

Oluşturulan arşiv yapısı iki mantıksal bölümden oluşur:

1. Organizasyon Bölümü: İlk 10 bayt, bu bölümün toplam karakter uzunluğunu sabit genişlikte ASCII olarak tutar (Örn: 0000000125). Ardından gelen her dosya verisi |dosya_adi,izin,boyut| şablonuyla eklenir.

2. Veri Bölümü: Meta-veri alanı bittiği andan itibaren tüm kaynak dosyaların içerikleri hiçbir ayırıcı karakter kullanılmadan peş peşe arşive yazılır. Geri çıkarılırken organizasyon bölümündeki net boyut bilgisi kadar okuma yapılır.
