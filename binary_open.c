#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
 
 
int read_file(char* filename, char *buf, int size)
{
    FILE* fp;
    int read_len = 0;
 
    if ((fp = fopen(filename, "rb")) == NULL)
    {
        printf("file open error '%s'\n", filename);
        return -1;
    }
    read_len = fread(buf, sizeof(char), size, fp);
 
    fclose(fp);
 
    return 0;
 
}
 
int main()
{
    char filename[256] = "example2.docx";
    char write_data[20] = { 0xFF, 0xFD, 0xFC, 0xFB, 0xFA, };
    char read_data[20] = { 0x00, };
 

    read_file(filename, read_data, 5);
    for (int i = 0; i < 5; i++)
        printf("0x%X ", (unsigned char)read_data[i]);

    return 0;
}
