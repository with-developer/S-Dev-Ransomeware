#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define MAX_LEN 512


void directory_indexing(char *directory_list[], int directory_list_len){
    printf("directory_list_len: %d\n",directory_list_len);
    for (int i = 0; i < directory_list_len; i++){
        WIN32_FIND_DATAA data;
        printf("\nFind %s Directory: \n",directory_list[i]);

        HANDLE hFind = FindFirstFileA(directory_list[i], &data);
        printf("(d): %s\n",data.cFileName);

        while(FindNextFileA(hFind, &data)){
            if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)){
                printf("(d): %s\n",data.cFileName);
            }
            else if((data.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) && !(data.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)){
                printf("(f): %s\n",data.cFileName);
            }
        }
        FindClose(hFind);
        printf("\n\n");

    }
    

    return;
}


int main(){
    // 초기 설정
    char *directory_list[4] = {"C:\\Users\\user\\Documents\\*", "C:\\Users\\user\\Desktop\\*", "C:\\Users\\user\\Downloads\\*", ""};

    //Get User Name
    DWORD dwsize = MAX_LEN;
    char strUserName[MAX_LEN] = { 0 };

    int nError = GetUserNameA(strUserName, &dwsize);
    if (!nError) {
        printf("GetUserNameA function is ERROR\n");
        return -1;
    }
    printf("Get Username: %s\n", strUserName);

    // directory_list 4번째 인덱스 추가
    char temp_directory[MAX_LEN] = { 0 };
    sprintf(temp_directory, "C:\\Users\\%s\\Appdata\\Local\\Temp\\*", strUserName);
    directory_list[3] = temp_directory;
    

    
    
    directory_indexing(directory_list, sizeof(directory_list)/ sizeof(directory_list[0]));
    


    return 1;

}
