#include "tack/editor.h"
#include "test.h"

#include <stdlib.h>

static void load_text(Editor *e, const char *s)
{
    editor_init(e);
    buf_load_mem(&e->buf, s, strlen(s));
    editor_set_view(e, 10, 40);
}

void test_editor_arrows(void)
{
    Editor e;
    load_text(&e, "ab\ncd");
    ASSERT_EQ_INT("start y", 0, (int)e.cy);
    ASSERT_EQ_INT("start x", 0, (int)e.cx);
    editor_move_right(&e);
    ASSERT_EQ_INT("x 1", 1, (int)e.cx);
    editor_move_right(&e);
    editor_move_right(&e);
    ASSERT_EQ_INT("wrapped y", 1, (int)e.cy);
    ASSERT_EQ_INT("wrapped x", 0, (int)e.cx);
    editor_move_left(&e);
    ASSERT_EQ_INT("back y", 0, (int)e.cy);
    ASSERT_EQ_INT("back x", 2, (int)e.cx);
    editor_move_down(&e);
    ASSERT_EQ_INT("down y", 1, (int)e.cy);
    editor_free(&e);
}

void test_editor_home_end(void)
{
    Editor e;
    load_text(&e, "hello");
    editor_move_end(&e);
    ASSERT_EQ_INT("end", 5, (int)e.cx);
    editor_move_home(&e);
    ASSERT_EQ_INT("home", 0, (int)e.cx);
    editor_free(&e);
}

void test_editor_goal_column(void)
{
    Editor e;
    load_text(&e, "abcdef\nxy\n123456");
    editor_move_end(&e);
    ASSERT_EQ_INT("end of first", 6, (int)e.cx);
    editor_move_down(&e);
    ASSERT_EQ_INT("clamped to short line", 2, (int)e.cx);
    editor_move_down(&e);
    ASSERT_EQ_INT("restored goal", 6, (int)e.cx);
    editor_free(&e);
}

void test_editor_ctrl_q(void)
{
    Editor e;
    Event ev;
    load_text(&e, "x");
    ev.kind = EV_KEY;
    ev.key = KEY_CHAR;
    ev.ch = 'q';
    ev.mods = MOD_CTRL;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    ASSERT_EQ_INT("quit", 1, editor_handle_event(&e, &ev));
    ASSERT_EQ_INT("flag", 1, e.quit);
    editor_free(&e);
}

void test_editor_handle_arrows(void)
{
    Editor e;
    Event ev;
    load_text(&e, "one\ntwo");
    ev.kind = EV_KEY;
    ev.key = KEY_RIGHT;
    ev.ch = 0;
    ev.mods = 0;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    editor_handle_event(&e, &ev);
    ASSERT_EQ_INT("right", 1, (int)e.cx);
    ev.key = KEY_DOWN;
    editor_handle_event(&e, &ev);
    ASSERT_EQ_INT("down", 1, (int)e.cy);
    editor_free(&e);
}

void test_editor_scroll(void)
{
    Editor e;
    int i;
    editor_init(&e);
    for (i = 0; i < 30; i++) {
        buf_insert_cstr(&e.buf, (size_t)i, 0, "line");
        if (i < 29) {
            buf_split(&e.buf, (size_t)i, 4);
        }
    }
    editor_set_view(&e, 5, 20);
    e.cy = 20;
    editor_scroll_into_view(&e);
    ASSERT_TRUE("scrolled", e.row_off > 0);
    ASSERT_TRUE("cursor visible", e.cy >= e.row_off &&
                                      e.cy < e.row_off + (size_t)e.view_rows);
    editor_free(&e);
}

void test_editor_render(void)
{
    Editor e;
    Screen s;
    load_text(&e, "Hello");
    screen_init(&s);
    screen_resize(&s, 8, 40);
    editor_set_view(&e, 6, 40);
    editor_render(&e, &s);
    ASSERT_EQ_INT("title style", STYLE_TITLE, screen_get_style(&s, 0, 0));
    ASSERT_EQ_INT("status style", STYLE_STATUS, screen_get_style(&s, 7, 0));
    ASSERT_EQ_INT("H", (int)'H', (int)screen_get(&s, 1, 0));
    ASSERT_EQ_INT("e", (int)'e', (int)screen_get(&s, 1, 1));
    ASSERT_EQ_INT("cursor y", 1, s.cy);
    ASSERT_EQ_INT("cursor x", 0, s.cx);
    screen_free(&s);
    editor_free(&e);
}

void test_editor_type_and_undo(void)
{
    Editor e;
    size_t n = 0;
    char *s;
    Event ev;
    load_text(&e, "");
    editor_insert_text(&e, "ab", 2);
    editor_newline(&e);
    editor_insert_text(&e, "c", 1);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("typed", "ab\nc", s);
    free(s);
    ASSERT_EQ_INT("dirty", 1, e.dirty);
    editor_undo(&e);
    editor_undo(&e);
    editor_undo(&e);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("undone", "", s);
    free(s);
    editor_redo(&e);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("redone first", "ab", s);
    free(s);

    ev.kind = EV_KEY;
    ev.key = KEY_CHAR;
    ev.ch = 'z';
    ev.mods = MOD_CTRL;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    editor_handle_event(&e, &ev);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("ctrl+z", "", s);
    free(s);
    editor_free(&e);
}

void test_editor_backspace_delete(void)
{
    Editor e;
    size_t n = 0;
    char *s;
    load_text(&e, "abcd");
    e.cx = 2;
    editor_backspace(&e);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("backspace", "acd", s);
    free(s);
    editor_delete_forward(&e);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("delete", "ad", s);
    free(s);
    editor_free(&e);
}

void test_editor_backspace_join(void)
{
    Editor e;
    size_t n = 0;
    char *s;
    load_text(&e, "ab\ncd");
    e.cy = 1;
    e.cx = 0;
    editor_backspace(&e);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("join", "abcd", s);
    free(s);
    ASSERT_EQ_INT("cursor", 2, (int)e.cx);
    editor_free(&e);
}

void test_editor_enter(void)
{
    Editor e;
    Event ev;
    size_t n = 0;
    char *s;
    load_text(&e, "hi");
    e.cx = 1;
    ev.kind = EV_KEY;
    ev.key = KEY_ENTER;
    ev.ch = 0;
    ev.mods = 0;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    editor_handle_event(&e, &ev);
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("split", "h\ni", s);
    free(s);
    ASSERT_EQ_INT("y", 1, (int)e.cy);
    ASSERT_EQ_INT("x", 0, (int)e.cx);
    editor_free(&e);
}

void test_editor_word_move(void)
{
    Editor e;
    load_text(&e, "foo bar baz");
    editor_move_word_right(&e);
    ASSERT_EQ_INT("to bar", 4, (int)e.cx);
    editor_move_word_right(&e);
    ASSERT_EQ_INT("to baz", 8, (int)e.cx);
    editor_move_word_left(&e);
    ASSERT_EQ_INT("back bar", 4, (int)e.cx);
    editor_free(&e);
}
