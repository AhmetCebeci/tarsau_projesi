#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Hatalı kullanımları önlemek için minimum argüman kontrolü
    if (argc < 2) {
        printf("Kullanim hatasi! Lutfen -b veya -a girin.\n");
        return 1;
    }

    // ==========================================
    // BİRLEŞTİRME MODU (-b)
    // ==========================================
    if (strcmp(argv[1], "-b") == 0) {
        char *output_file = "a.sau"; // Varsayılan çıktı dosyası adı
        char *input_files[32];
        int input_count = 0;

        // Komut satırı argümanlarını ayrıştır (-o parametresini ve giriş dosyalarını yakala)
        for(int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                // -o parametresinden sonraki argümanı çıktı dosyası adı olarak belirle
                if (i + 1 < argc) { 
                    output_file = argv[i+1]; 
                    i++; // Çıktı dosya adı okunduğu için bir sonraki indeksi atla
                }
            } else {
                // Maksimum 32 dosya sınırını kontrol et
                if (input_count < 32) { 
                    input_files[input_count++] = argv[i]; 
                } else { 
                    printf("Hata: Giris dosyasi sayisi en fazla 32 olabilir.\n"); 
                    return 0; 
                }
            }
        }

        // Paketlenecek dosya girilmediyse programı sonlandır
        if (input_count == 0) {
            printf("Hata: Paketlenecek dosya girilmedi!\n");
            return 0;
        }

        char header_content[4096] = ""; 
        long total_file_size = 0;

        // Giriş dosyalarını doğrula ve arşiv başlık (header) bilgisini hazırla
        for(int i = 0; i < input_count; i++) {
            // Sadece .txt ve .dat uzantılı metin dosyalarına izin ver
            if (!strstr(input_files[i], ".txt") && !strstr(input_files[i], ".dat")) {
                printf("%s giris dosyasinin formati uyumsuzdur!\n", input_files[i]);
                return 0; 
            }

            // Dosya boyutunu ve izinlerini okumak için stat sistem çağrısını kullan
            struct stat file_stat;
            if (stat(input_files[i], &file_stat) == -1) {
                printf("Hata: %s dosyasi bulunamadi!\n", input_files[i]);
                return 0;
            }
            
            // Dosya bilgilerini '|dosya_adi,izin,boyut|' formatında header'a ekle
            // file_stat.st_mode & 0777 ile dosyanın rwx izinleri octal olarak alınır
            char record[256];
            sprintf(record, "|%s,%o,%ld|", input_files[i], file_stat.st_mode & 0777, file_stat.st_size);
            strcat(header_content, record);
            
            total_file_size += file_stat.st_size; // Toplam boyutu hesapla
        }

        // Toplam boyut 200 MB sınırını aşıyor mu kontrol et
        if (total_file_size > 200 * 1024 * 1024) { 
            printf("Hata: Giris dosyalarinin toplami 200 MB'i gecemez.\n");
            return 0;
        }

        // Hedef arşiv dosyasını oluştur
        FILE *archive = fopen(output_file, "w");
        if (!archive) {
            printf("Hata: Arsiv dosyasi olusturulamadi!\n");
            return 0;
        }

        // İlk 10 bayta header kısmının toplam karakter uzunluğunu sabit genişlikte (010d) yaz
        int header_length = strlen(header_content);
        fprintf(archive, "%010d", header_length); 
        fprintf(archive, "%s", header_content); // Header içeriğini arşive yaz

        // Giriş dosyalarının içeriklerini sırayla arşive ekle
        for(int i = 0; i < input_count; i++) {
            FILE *source = fopen(input_files[i], "r");
            if (source) {
                char ch;
                // Dosyayı karakter karakter okuyup doğrudan arşive kopyala
                while ((ch = fgetc(source)) != EOF) { fputc(ch, archive); }
                fclose(source);
            }
        }
        fclose(archive);
        printf("Dosyalar birlesitirildi.\n");
    } 
    // ==========================================
    // ARŞİVDEN ÇIKARMA MODU (-a)
    // ==========================================
    else if (strcmp(argv[1], "-a") == 0) {
        // Argüman sayısı kontrolü (en fazla bir arşiv adı ve bir hedef dizin gelebilir)
        if (argc < 3 || argc > 4) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            return 0;
        }

        char *archive_name = argv[2];
        
        // Giriş dosyasının .sau uzantılı olup olmadığını doğrula
        if (!strstr(archive_name, ".sau")) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            return 0;
        }

        char *target_dir = "."; // Varsayılan hedef dizin: Geçerli klasör

        // Eğer kullanıcı özel bir hedef dizin belirttiyse
        if (argc == 4) {
            target_dir = argv[3];
            struct stat st = {0};
            // Belirtilen dizin mevcut değilse otomatik olarak oluştur
            if (stat(target_dir, &st) == -1) {
                if (mkdir(target_dir, 0755) == -1) {
                    printf("Hata: Dizin olusturulamadi!\n");
                    return 0;
                }
            }
        }

        // Arşiv dosyasını okuma modunda aç
        FILE *archive = fopen(archive_name, "r");
        if (!archive) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            return 0;
        }

        // İlk 10 bayttan header uzunluğunu oku
        char header_len_str[11] = {0};
        if (fread(header_len_str, 1, 10, archive) != 10) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            fclose(archive);
            return 0;
        }

        int header_len = atoi(header_len_str);
        if (header_len <= 0) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            fclose(archive);
            return 0;
        }
        
        // Header içeriğini okumak için dinamik bellek tahsis et
        char *header_content = malloc(header_len + 1);
        if (fread(header_content, 1, header_len, archive) != header_len) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(header_content);
            fclose(archive);
            return 0;
        }
        header_content[header_len] = '\0';

        // Basit format doğrulaması (Header '|' ile başlamalı)
        if (header_content[0] != '|') {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            free(header_content);
            fclose(archive);
            return 0;
        }

        // Header içeriğini parçalayarak her bir dosyanın kaydını oku
        char *record = strtok(header_content, "|");
        while (record != NULL) {
            char filename[256];
            int permissions;
            long size;
            
            // Dosya adı, izin bitleri ve boyut bilgisini ayrıştır
            if (sscanf(record, "%[^,],%o,%ld", filename, &permissions, &size) == 3) {
                char full_path[512];
                sprintf(full_path, "%s/%s", target_dir, filename); // Tam dosya yolunu oluştur

                FILE *out = fopen(full_path, "w");
                if (out) {
                    // Belirtilen dosya boyutu kadar karakteri arşivden okuyup yeni dosyaya yaz
                    for (long k = 0; k < size; k++) {
                        int ch = fgetc(archive);
                        if (ch != EOF) fputc(ch, out);
                    }
                    fclose(out);
                    
                    // Dosyanın orijinal Linux izinlerini geri yükle
                    chmod(full_path, permissions); 
                }
            }
            record = strtok(NULL, "|"); // Sonraki dosya kaydına geç
        }

        // Kaynakları temizle
        free(header_content);
        fclose(archive);
        printf("%s dizininde dosyalar acildi.\n", target_dir);
    } 
    else {
        printf("Gecersiz parametre!\n");
    }

    return 0;
}