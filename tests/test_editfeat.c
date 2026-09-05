#include "missnotepad/editor.h"
#include "missnotepad/search.h"
#include "missnotepad/wrap.h"
#include "test.h"

#include <stdlib.h>

void test_search_find(void)
{
    Buffer b;
    size_t row = 0, col = 0;
    buf_init(&b);
    buf_load_mem(&b, "hello\nthere hello", 17);
    ASSERT_TRUE("find", search_find(&b, "hello", 0, 0, &row, &col));
    ASSERT_EQ_INT("row0", 0, (int)row);
    ASSERT_EQ_INT("col0", 0, (int)col);
    ASSERT_TRUE("find2", search_find(&b, "hello", 0, 1, &row, &col));
    ASSERT_EQ_INT("row1", 1, (int)row);
    ASSERT_EQ_INT("col1", 6, (int)col);
    buf_free(&b);
}

void test_search_replace_all(void)
{
    Buffer b;
    int n = 0;
    size_t len = 0;
    char *s;
    buf_init(&b);
    buf_load_mem(&b, "aa x aa", 7);
    ASSERT_EQ_INT("ok", 0, search_replace_all(&b, "aa", "b", &n));
    ASSERT_EQ_INT("count", 2, n);
    s = buf_to_string(&b, &len);
    ASSERT_STREQ("replaced", "b x b", s);
    free(s);
    buf_free(&b);
}

void test_wrap_starts(void)
{
    size_t starts[8];
    size_t n = wrap_line_starts("hello world", 11, 4, 8, starts, 8);
    ASSERT_TRUE("at least 2", n >= 2);
    ASSERT_EQ_INT("first", 0, (int)starts[0]);
}

void test_editor_cut_copy_paste(void)
{
    Editor e;
    size_t n = 0;
    char *s;
    editor_init(&e);
    buf_load_mem(&e.buf, "abcdef", 6);
    e.sel_on = 1;
    e.sy = 0;
    e.sx = 1;
    e.cy = 0;
    e.cx = 4;
    ASSERT_EQ_INT("copy", 0, editor_copy(&e));
    ASSERT_TRUE("clip", e.clip != NULL && memcmp(e.clip, "bcd", 3) == 0);
    ASSERT_EQ_INT("cut", 0, editor_cut(&e));
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("after cut", "aef", s);
    free(s);
    e.cx = 1;
    ASSERT_EQ_INT("paste", 0, editor_paste(&e));
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("pasted", "abcdef", s);
    free(s);
    editor_free(&e);
}

void test_editor_select_all_and_find(void)
{
    Editor e;
    editor_init(&e);
    buf_load_mem(&e.buf, "one two one", 11);
    editor_select_all(&e);
    ASSERT_TRUE("sel", e.sel_on);
    ASSERT_EQ_INT("end", 11, (int)e.cx);
    snprintf(e.find_text, sizeof(e.find_text), "one");
    e.cy = 0;
    e.cx = 0;
    e.sel_on = 0;
    ASSERT_TRUE("found", editor_find_next(&e));
    ASSERT_EQ_INT("at 0 or later", 0, (int)e.sy);
    editor_free(&e);
}

void test_editor_replace_all(void)
{
    Editor e;
    size_t n = 0;
    char *s;
    editor_init(&e);
    buf_load_mem(&e.buf, "cat cat", 7);
    ASSERT_EQ_INT("rep", 0, editor_replace_all(&e, "cat", "dog"));
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("dogs", "dog dog", s);
    free(s);
    editor_free(&e);
}

void test_editor_word_wrap_render(void)
{
    Editor e;
    Screen s;
    editor_init(&e);
    buf_load_mem(&e.buf, "abcdefghijKLMNOPQRST", 20);
    e.word_wrap = 1;
    e.show_linenum = 0;
    editor_set_view(&e, 6, 10);
    screen_init(&s);
    screen_resize(&s, 8, 10);
    editor_render(&e, &s);
    ASSERT_EQ_INT("a", (int)'a', (int)screen_get(&s, 1, 0));
    ASSERT_TRUE("wrapped something", screen_get(&s, 2, 0) != (uint32_t)' ');
    screen_free(&s);
    editor_free(&e);
}

void test_editor_word_wrap_click(void)
{
    Editor e;
    editor_init(&e);
    /* Width 10 wraps line 0 into visual rows "aaaa " and "bbbbb cccc";
       line 1 ("second") is the third visual row. */
    buf_load_mem(&e.buf, "aaaa bbbbb cccc\nsecond", 22);
    e.word_wrap = 1;
    e.show_linenum = 0;
    editor_set_view(&e, 6, 10);

    editor_click(&e, 1, 2);
    ASSERT_EQ_INT("click: wrapped row file line", 0, (int)e.cy);
    ASSERT_EQ_INT("click: col within segment", 7, (int)e.cx);

    editor_click(&e, 2, 3);
    ASSERT_EQ_INT("click: next file line", 1, (int)e.cy);
    ASSERT_EQ_INT("click: col on next line", 3, (int)e.cx);

    editor_click(&e, 1, 0);
    ASSERT_EQ_INT("click: start of wrapped row", 5, (int)e.cx);

    editor_click(&e, 1, 12);
    ASSERT_EQ_INT("click: past row end clamps", 0, (int)e.cy);
    ASSERT_EQ_INT("click: clamped to row end", 15, (int)e.cx);

    /* Clamping on a non-final visual row must stay on that row (byte 4 is
       the trailing space of "aaaa ") instead of jumping to the next one. */
    editor_click(&e, 0, 12);
    ASSERT_EQ_INT("click: first row past end", 0, (int)e.cy);
    ASSERT_EQ_INT("click: stays on first row", 4, (int)e.cx);

    editor_click(&e, 5, 0);
    ASSERT_EQ_INT("click: below document", 1, (int)e.cy);
    ASSERT_EQ_INT("click: end of document", 6, (int)e.cx);

    editor_free(&e);
}

void test_editor_word_wrap_down_insert(void)
{
    Editor e;
    Screen s;
    Event ev;
    editor_init(&e);
    buf_load_mem(&e.buf, "abcdefghijKLMNOPQRST\nxyz", 24);
    e.word_wrap = 1;
    e.show_linenum = 0;
    editor_set_view(&e, 6, 10);
    ev.kind = EV_KEY;
    ev.key = KEY_DOWN;
    ev.ch = 0;
    ev.mods = 0;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    editor_handle_event(&e, &ev);
    ASSERT_EQ_INT("same file line", 0, (int)e.cy);
    ASSERT_EQ_INT("next wrapped segment", 10, (int)e.cx);
    ev.key = KEY_CHAR;
    ev.ch = 'Q';
    editor_handle_event(&e, &ev);
    ASSERT_EQ_INT("inserted in wrapped line", 11, (int)e.cx);
    ev.key = KEY_DOWN;
    ev.ch = 0;
    editor_handle_event(&e, &ev);
    ev.key = KEY_CHAR;
    ev.ch = 'R';
    editor_handle_event(&e, &ev);
    screen_init(&s);
    screen_resize(&s, 8, 10);
    editor_render(&e, &s);
    ASSERT_EQ_INT("inserted on wrapped line", (int)'R',
                  (int)screen_get(&s, 3, 1));
    ASSERT_EQ_INT("first insert on wrapped line", (int)'Q',
                  (int)screen_get(&s, 2, 0));
    screen_free(&s);
    editor_free(&e);
}

void test_editor_word_wrap_down_screen_line(void)
{
    Editor e;
    Screen s;
    editor_init(&e);
    /* Text width 10 wraps line 0 at the word boundary into the visual
       rows "aaaa " (bytes 0-4) and "bbbbb cccc" (bytes 5-14). */
    buf_load_mem(&e.buf, "aaaa bbbbb cccc\nsecond", 22);
    e.word_wrap = 1;
    e.show_linenum = 0;
    editor_set_view(&e, 6, 10);

    e.cy = 0;
    e.cx = 2; /* screen row 0, screen x 2 */
    editor_move_down(&e);
    ASSERT_EQ_INT("same file line", 0, (int)e.cy);
    ASSERT_EQ_INT("same screen x, next screen line", 7, (int)e.cx);

    editor_move_up(&e);
    ASSERT_EQ_INT("up: previous screen line", 2, (int)e.cx);

    e.cx = 13; /* last screen row of line 0, screen x 8 */
    editor_move_down(&e);
    ASSERT_EQ_INT("down from last screen row", 1, (int)e.cy);

    editor_move_up(&e);
    ASSERT_EQ_INT("up: back to line 0", 0, (int)e.cy);
    ASSERT_EQ_INT("up: same screen x", 11, (int)e.cx);

    editor_move_up(&e);
    ASSERT_EQ_INT("up: clamped to row end", 5, (int)e.cx);

    editor_move_up(&e);
    ASSERT_EQ_INT("up: previous screen row", 0, (int)e.cx);

    editor_move_up(&e);
    ASSERT_EQ_INT("up: stays at top", 0, (int)e.cx);

    /* Rendered layout must match the navigation segments. */
    screen_init(&s);
    screen_resize(&s, 8, 10);
    e.cy = 0;
    e.cx = 7; /* screen row 1, screen x 2 */
    editor_render(&e, &s);
    ASSERT_EQ_INT("screen row 0", (int)'a', (int)screen_get(&s, 1, 0));
    ASSERT_EQ_INT("screen row 1", (int)'b', (int)screen_get(&s, 2, 0));
    ASSERT_EQ_INT("screen row 2", (int)'s', (int)screen_get(&s, 3, 0));
    ASSERT_EQ_INT("cursor screen row", 2, s.cy);
    ASSERT_EQ_INT("cursor screen x", 2, s.cx);
    screen_free(&s);
    editor_free(&e);
}
