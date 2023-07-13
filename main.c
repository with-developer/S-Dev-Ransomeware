#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_LEN 512

// 경로별 구조체 생성
struct directory_document{
    char *file_name;   // 파일명
    char *file_type;   // 파일인지, 디렉터리인지
    int count;         // 파일 및 디렉토리 수
};

struct directory_document* document_dir;
struct directory_document* desktop_dir;
struct directory_document* downloads_dir;
struct directory_document* temp_dir;

void directory_indexing(char *directory_list[], int directory_list_len){
    for (int i = 0; i < directory_list_len; i++){
        int file_count = 0;
        WIN32_FIND_DATAA data;
        printf("\nFind %s Directory: \n",directory_list[i]);

        HANDLE hFind = FindFirstFileA(directory_list[i], &data);

        struct directory_document** target_dir = NULL;

        switch(i) {
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
            if(!(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && strcmp(data.cFileName, "..") != 0 && strcmp(data.cFileName, ".") != 0){
                file_count++;

                *target_dir = realloc(*target_dir, sizeof(struct directory_document) * file_count);

                (*target_dir)[file_count - 1].file_name = strdup(data.cFileName);
                (*target_dir)[file_count - 1].file_type = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? "Directory" : "File";

                printf("%s: %s\n", (*target_dir)[file_count - 1].file_type, (*target_dir)[file_count - 1].file_name);
            }
        } while(FindNextFileA(hFind, &data));

        FindClose(hFind);

        printf("file count: %d\n\n",file_count);

        (*target_dir)[0].count = file_count;
    }

    return;
}

void print_directory_contents(struct directory_document* dir) {
    for(int i = 0; i < dir[0].count; i++) {
        printf("File Name: %s, File Type: %s\n", dir[i].file_name, dir[i].file_type);
    }
}

int main(){
    char *directory_list[4] = {"C:\\Users\\user\\Documents\\*", "C:\\Users\\user\\Desktop\\*", "C:\\Users\\user\\Downloads\\*", ""};

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

    directory_indexing(directory_list, sizeof(directory_list)/ sizeof(directory_list[0]));

    printf("\nDocument Directory:\n");
    print_directory_contents(document_dir);
    
    printf("\nDesktop Directory:\n");
    print_directory_contents(desktop_dir);
    
    printf("\nDownloads Directory:\n");
    print_directory_contents(downloads_dir);
    
    printf("\nTemp Directory:\n");
    print_directory_contents(temp_dir);

    // Don't forget to free your allocated memory!
    // Free each string and the struct itself.

    // add your free statements here

    return 1;
}
