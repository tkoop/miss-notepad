#include "tack/fileio.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void test_file_read_all(void)
{
    char path[] = "/tmp/tack-test-XXXXXX";
    int fd = mkstemp(path);
    char *data = NULL;
    size_t len = 0;
    FILE *fp;
    int rc;

    ASSERT_TRUE("mkstemp", fd >= 0);
    if (fd < 0) {
        return;
    }
    close(fd);
    fp = fopen(path, "wb");
    ASSERT_NOT_NULL("fopen", fp);
    if (fp == NULL) {
        return;
    }
    fwrite("hello\nworld", 1, 11, fp);
    fclose(fp);

    rc = file_read_all(path, &data, &len);
    ASSERT_EQ_INT("ok", 0, rc);
    ASSERT_EQ_INT("len", 11, (int)len);
    ASSERT_TRUE("content", data != NULL && memcmp(data, "hello\nworld", 11) == 0);
    free(data);
    unlink(path);
}

void test_file_read_missing(void)
{
    char *data = NULL;
    size_t len = 0;
    int rc = file_read_all("/tmp/tack-does-not-exist-xyzzy", &data, &len);
    ASSERT_EQ_INT("not found", 1, rc);
    ASSERT_NULL("no data", data);
}
