#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define CHUNK 16384 

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *source = fopen(argv[1], "rb");
    if (!source) {
        perror("fopen");
        return 1;
    }

    FILE *dest = fopen(argv[2], "wb");
    if (!dest) {
        perror("fopen");
        fclose(source);
        return 1;
    }

    int ret;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    memset(&strm, 0, sizeof(strm));
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        fprintf(stderr, "inflateInit failed with error %d\n", ret);
        fclose(source);
        fclose(dest);
        return 1;
    }

    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            perror("fread");
            inflateEnd(&strm);
            fclose(source);
            fclose(dest);
            return 1;
        }
        if (strm.avail_in == 0)
            break;

        strm.next_in = in;

        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            switch (ret) {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&strm);
                    fclose(source);
                    fclose(dest);
                    return 1;
            }
            unsigned have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                perror("fwrite");
                inflateEnd(&strm);
                fclose(source);
                fclose(dest);
                return 1;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    fclose(source);
    fclose(dest);

    return 0;
}

//g++ zlib_decompress.cpp -o zlib_decompress -lz -g -O0