#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>

//g++ archive_read.cpp -o archive_read -g -O0 -larchive
int main() {
    struct archive *a;
    struct archive_entry *entry;
    int r;

    // 创建一个archive对象，用于读取压缩文件
    a = archive_read_new();
    // 设置archive对象的格式为gzip
    // archive_read_support_format_raw(a);
    // archive_read_support_format_all(a);
    // archive_read_support_filter_none(a);
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    // archive_read_support_compression_all(a);

    // 打开要读取的压缩文件
    r = archive_read_open_filename(a, "/home/gpadmin/000000_0.deflate", 10240); // 替换为你的文件名
    if (r != ARCHIVE_OK) {
        printf("Failed to open archive.\n");
        return 1;
    }

    // 读取压缩文件中的每个文件条目
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        printf("File name: %s\n", archive_entry_pathname(entry));

        // 解压缩文件内容并打印到标准输出
        // 这里可以根据需要进行处理，比如写入文件
        const void *buff;
        size_t size;
        off_t offset;
        while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
            printf("Data: %s", (const char *)buff);
        }
    }

    // 关闭archive对象
    archive_read_close(a);
    archive_read_free(a);

    return 0;
}