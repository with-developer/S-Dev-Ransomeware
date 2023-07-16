#include <stdio.h>
#include "miniz.h"

int main() {
    const char* src = "Hello, World!";
    const size_t src_size = strlen(src) + 1;

    mz_ulong compressed_size = mz_compressBound(src_size);
    unsigned char* compressed_buf = (unsigned char*)malloc(compressed_size);

    int status = mz_compress(compressed_buf, &compressed_size, (const unsigned char*)src, src_size);
    if (status == Z_OK) {
        printf("Compressed size: %lu\n", compressed_size);
        printf("Compressed data: ");
        for (mz_ulong i = 0; i < compressed_size; ++i) {
            printf("%02x ", compressed_buf[i]);
        }
        printf("\n");
    } else {
        printf("Compression failed with status: %d\n", status);
    }

    free(compressed_buf);

    return 0;
}
