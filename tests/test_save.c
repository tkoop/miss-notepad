#include "missnotepad/editor.h"
#include "missnotepad/fileio.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void test_file_write_and_read(void)
{
    char path[] = "/tmp/missnotepad-write-XXXXXX";
    int fd = mkstemp(path);
    char *data = NULL;
    size_t len = 0;

    ASSERT_TRUE("mkstemp", fd >= 0);
    if (fd < 0) {
        return;
    }
    close(fd);
    ASSERT_EQ_INT("write", 0, file_write_all(path, "abc\nxyz", 7));
    ASSERT_EQ_INT("read", 0, file_read_all(path, &data, &len));
    ASSERT_EQ_INT("len", 7, (int)len);
    ASSERT_TRUE("data", data != NULL && memcmp(data, "abc\nxyz", 7) == 0);
    free(data);
    unlink(path);
}

void test_editor_save_roundtrip(void)
{
    Editor e;
    char path[] = "/tmp/missnotepad-save-XXXXXX";
    int fd = mkstemp(path);
    char *data = NULL;
    size_t len = 0;

    ASSERT_TRUE("mkstemp", fd >= 0);
    if (fd < 0) {
        return;
    }
    close(fd);
    unlink(path);

    editor_init(&e);
    editor_insert_text(&e, "saved text", 10);
    ASSERT_EQ_INT("dirty", 1, e.dirty);
    ASSERT_EQ_INT("save as", 0, editor_save_as(&e, path));
    ASSERT_EQ_INT("clean", 0, e.dirty);
    ASSERT_STREQ("name", path, e.filename);
    ASSERT_EQ_INT("read", 0, file_read_all(path, &data, &len));
    ASSERT_TRUE("content", data != NULL && memcmp(data, "saved text", 10) == 0);
    free(data);
    editor_free(&e);
    unlink(path);
}

void test_editor_save_untitled(void)
{
    Editor e;
    editor_init(&e);
    editor_insert_text(&e, "x", 1);
    ASSERT_EQ_INT("no name", -1, editor_save(&e));
    ASSERT_TRUE("message", e.status_msg[0] != '\0');
    editor_free(&e);
}

void test_editor_new(void)
{
    Editor e;
    editor_init(&e);
    editor_insert_text(&e, "gone", 4);
    ASSERT_EQ_INT("new", 0, editor_new(&e));
    ASSERT_EQ_INT("empty", 1, (int)buf_line_count(&e.buf));
    ASSERT_EQ_INT("len", 0, (int)buf_line_len(&e.buf, 0));
    ASSERT_NULL("no file", e.filename);
    ASSERT_EQ_INT("clean", 0, e.dirty);
    editor_free(&e);
}

void test_editor_ctrl_s(void)
{
    Editor e;
    Event ev;
    char path[] = "/tmp/missnotepad-ctrls-XXXXXX";
    int fd = mkstemp(path);

    ASSERT_TRUE("mkstemp", fd >= 0);
    if (fd < 0) {
        return;
    }
    close(fd);

    editor_init(&e);
    editor_load_path(&e, path);
    editor_insert_text(&e, "ok", 2);
    ev.kind = EV_KEY;
    ev.key = KEY_CHAR;
    ev.ch = 's';
    ev.mods = MOD_CTRL;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    editor_handle_event(&e, &ev);
    ASSERT_EQ_INT("saved", 0, e.dirty);
    editor_free(&e);
    unlink(path);
}
