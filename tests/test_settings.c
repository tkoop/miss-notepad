#include "tack/editor.h"
#include "tack/settings.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void test_settings_defaults(void)
{
    Settings s;
    settings_defaults(&s);
    ASSERT_EQ_INT("linenum on", 1, s.show_linenum);
    ASSERT_EQ_INT("wrap off", 0, s.word_wrap);
    ASSERT_EQ_INT("tab 4", 4, s.tabstop);
    ASSERT_EQ_INT("notepad", THEME_NOTEPAD, s.theme);
}

void test_settings_parse_format(void)
{
    Settings s;
    const char *text = "show_linenum=0\nword_wrap=1\ntabstop=8\nkey_theme=nano\n";
    size_t n = 0;
    char *out;
    settings_parse(&s, text, strlen(text));
    ASSERT_EQ_INT("linenum", 0, s.show_linenum);
    ASSERT_EQ_INT("wrap", 1, s.word_wrap);
    ASSERT_EQ_INT("tab", 8, s.tabstop);
    ASSERT_EQ_INT("theme", THEME_NANO, s.theme);
    out = settings_format(&s, &n);
    ASSERT_NOT_NULL("format", out);
    ASSERT_TRUE("has nano", strstr(out, "key_theme=nano") != NULL);
    ASSERT_TRUE("has wrap", strstr(out, "word_wrap=1") != NULL);
    free(out);
}

void test_settings_roundtrip_file(void)
{
    Settings a, b;
    char path[] = "/tmp/tack-cfg-XXXXXX";
    int fd = mkstemp(path);
    ASSERT_TRUE("mkstemp", fd >= 0);
    if (fd < 0) {
        return;
    }
    close(fd);
    settings_defaults(&a);
    a.show_linenum = 0;
    a.theme = THEME_EMACS;
    ASSERT_EQ_INT("save", 0, settings_save(&a, path));
    ASSERT_EQ_INT("load", 0, settings_load(&b, path));
    ASSERT_EQ_INT("linenum", 0, b.show_linenum);
    ASSERT_EQ_INT("theme", THEME_EMACS, b.theme);
    unlink(path);
}

void test_editor_linenum_toggle(void)
{
    Editor e;
    Screen s;
    editor_init(&e);
    buf_load_mem(&e.buf, "Hi", 2);
    editor_set_view(&e, 6, 40);
    ASSERT_TRUE("default gutter", editor_gutter_width(&e) > 0);
    screen_init(&s);
    screen_resize(&s, 8, 40);
    editor_render(&e, &s);
    ASSERT_EQ_INT("pipe visible", (int)'|',
                  (int)screen_get(&s, 1, editor_gutter_width(&e) - 1));
    editor_toggle_line_numbers(&e);
    ASSERT_EQ_INT("off", 0, editor_gutter_width(&e));
    editor_render(&e, &s);
    ASSERT_EQ_INT("H at 0", (int)'H', (int)screen_get(&s, 1, 0));
    screen_free(&s);
    editor_free(&e);
}

void test_editor_apply_settings(void)
{
    Editor e;
    Settings s;
    editor_init(&e);
    settings_defaults(&s);
    s.show_linenum = 0;
    s.tabstop = 8;
    editor_apply_settings(&e, &s);
    ASSERT_EQ_INT("off", 0, e.show_linenum);
    ASSERT_EQ_INT("tab", 8, e.tabstop);
    editor_free(&e);
}
