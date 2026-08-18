#include "tack/fileio.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int file_read_all(const char *path, char **data, size_t *len)
{
    FILE *fp;
    struct stat st;
    char *buf;
    size_t nread;

    if (path == NULL || data == NULL || len == NULL) {
        errno = EINVAL;
        return -1;
    }
    *data = NULL;
    *len = 0;

    if (stat(path, &st) != 0) {
        return errno == ENOENT ? 1 : -1;
    }
    if (S_ISDIR(st.st_mode)) {
        errno = EISDIR;
        return -1;
    }
    fp = fopen(path, "rb");
    if (fp == NULL) {
        return errno == ENOENT ? 1 : -1;
    }
    buf = malloc((size_t)st.st_size + 1);
    if (buf == NULL) {
        fclose(fp);
        return -1;
    }
    nread = fread(buf, 1, (size_t)st.st_size, fp);
    if (ferror(fp)) {
        free(buf);
        fclose(fp);
        return -1;
    }
    fclose(fp);
    buf[nread] = '\0';
    *data = buf;
    *len = nread;
    return 0;
}

int file_write_all(const char *path, const char *data, size_t len)
{
    FILE *fp;
    size_t n;

    if (path == NULL) {
        errno = EINVAL;
        return -1;
    }
    fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }
    if (len > 0 && data != NULL) {
        n = fwrite(data, 1, len, fp);
        if (n != len) {
            fclose(fp);
            return -1;
        }
    }
    if (fclose(fp) != 0) {
        return -1;
    }
    return 0;
}
