#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Kullanim hatasi! Lutfen -b veya -a girin.\n");
        return 1;
    }

    // ==========================================
    // BİRLEŞTİRME MODU (-b)
    // ==========================================
    if (strcmp(argv[1], "-b") == 0) {
        char *output_file = "a.sau";
        char *input_files[32];
        int input_count = 0;

        for(int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) { output_file = argv[i+1]; i++; }
            } else {
                if (input_count < 32) { input_files[input_count++] = argv[i]; } 
                else { printf("Hata: En fazla 32 dosya girebilirsiniz!\n"); return 1; }
            }
        }

        char header_content[4096] = ""; 
        long total_file_size = 0;

        for(int i = 0; i < input_count; i++) {
            if (!strstr(input_files[i], ".txt") && !strstr(input_files[i], ".dat")) {
                printf("%s giris dosyasinin formati uyumsuzdur!\n", input_files[i]);
                return 1;
            }

            struct stat file_stat;
            if (stat(input_files[i], &file_stat) == -1) {
                printf("Hata: %s dosyasi bulunamadi!\n", input_files[i]);
                return 1;
            }
            
            char record[256];
            sprintf(record, "|%s,%o,%ld|", input_files[i], file_stat.st_mode & 0777, file_stat.st_size);
            strcat(header_content, record);
            total_file_size += file_stat.st_size;
        }

        if (total_file_size > 200 * 1024 * 1024) { 
            printf("Hata: Dosyalarin toplami 200 MB'i gecemez!\n");
            return 1;
        }

        FILE *archive = fopen(output_file, "w");
        if (!archive) {
            printf("Hata: Arsiv dosyasi olusturulamadi!\n");
            return 1;
        }

        int header_length = strlen(header_content);
        fprintf(archive, "%010d", header_length); 
        fprintf(archive, "%s", header_content);

        for(int i = 0; i < input_count; i++) {
            FILE *source = fopen(input_files[i], "r");
            if (source) {
                char ch;
                while ((ch = fgetc(source)) != EOF) { fputc(ch, archive); }
                fclose(source);
            }
        }
        fclose(archive);
        printf("Dosyalar birlestirildi.\n");
    } 
    // ==========================================
    // ARŞİVDEN ÇIKARMA MODU (-a)
    // ==========================================
    else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3) {
            printf("Hata: Arsiv dosyasi adi girilmedi!\n");
            return 1;
        }

        char *archive_name = argv[2];
        char *target_dir = "."; // Eğer dizin girilmezse bulunduğu yere çıkartır

        // Eğer hedef bir klasör girildiyse
        if (argc >= 4) {
            target_dir = argv[3];
            struct stat st = {0};
            // Klasör yoksa oluştur (0700 izniyle)
            if (stat(target_dir, &st) == -1) {
                mkdir(target_dir, 0700); 
            }
        }

        FILE *archive = fopen(archive_name, "r");
        if (!archive) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            return 1;
        }

        // İlk 10 baytı (içindekiler boyutunu) oku
        char header_len_str[11] = {0};
        if (fread(header_len_str, 1, 10, archive) != 10) {
            printf("Arsiv dosyasi uygunsuz veya bozuk!\n");
            fclose(archive);
            return 1;
        }

        int header_len = atoi(header_len_str);
        
        // İçindekiler listesini oku
        char *header_content = malloc(header_len + 1);
        fread(header_content, 1, header_len, archive);
        header_content[header_len] = '\0';

        // Dosya bilgilerini parçala ve dosyaları oluştur
        char *record = strtok(header_content, "|");

        while (record != NULL) {
            char filename[256];
            int permissions;
            long size;
            
            // Formatı çöz: DosyaAdı,İzinler,Boyut
            sscanf(record, "%[^,],%o,%ld", filename, &permissions, &size);

            char full_path[512];
            sprintf(full_path, "%s/%s", target_dir, filename);

            FILE *out = fopen(full_path, "w");
            if (out) {
                for (long k = 0; k < size; k++) {
                    fputc(fgetc(archive), out);
                }
                fclose(out);
                
                // Orijinal dosya izinlerini (okuma/yazma/çalıştırma) geri yükle
                chmod(full_path, permissions); 
            }
            record = strtok(NULL, "|");
        }

        free(header_content);
        fclose(archive);
        printf("%s dizininde dosyalar acildi.\n", target_dir);
    } 
    else {
        printf("Gecersiz parametre!\n");
    }

    return 0;
}
