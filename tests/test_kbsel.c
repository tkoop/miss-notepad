/* Keyboard text selection: Shift+arrows select, Shift+Ctrl+arrows select
 * whole words, typing replaces the selection, plain arrows clear it.
 * Events are fed as raw terminal bytes through event_parse, so the exact
 * escape sequences terminals send are covered too. */
#include "missnotepad/editor.h"
#include "missnotepad/keys.h"
#include "test.h"

static int feed(Editor *e, const char *bytes)
{
    Event ev;
    int used = event_parse((const unsigned char *)bytes, strlen(bytes), 0, &ev);
    if (used <= 0) {
        return -1;
    }
    editor_handle_event(e, &ev);
    return 0;
}

/* Selection content via the clipboard buffer. */
static void selected(Editor *e, char *out, size_t n)
{
    editor_copy(e);
    if (e->clip != NULL) {
        snprintf(out, n, "%.*s", (int)e->clip_len, e->clip);
    } else {
        snprintf(out, n, "%s", "");
    }
}

static void place_caret(Editor *e, size_t row, size_t col)
{
    e->sel_on = 0;
    e->sy = row;
    e->sx = col;
    e->cy = row;
    e->cx = col;
    e->goal_col = (int)col;
}

void test_kbsel_chars(void)
{
    Editor e;
    char got[64];
    editor_init(&e);
    buf_load_mem(&e.buf, "alpha beta gamma", 16);
    place_caret(&e, 0, 0);
    feed(&e, "\033[1;2C"); /* Shift+Right */
    feed(&e, "\033[1;2C");
    feed(&e, "\033[1;2C");
    ASSERT_EQ_INT("sel on", 1, e.sel_on);
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("chars selected", "alp", got);
    feed(&e, "\033[C"); /* plain Right */
    ASSERT_EQ_INT("sel cleared", 0, e.sel_on);
    editor_free(&e);
}

void test_kbsel_word_right(void)
{
    Editor e;
    char got[64];
    editor_init(&e);
    buf_load_mem(&e.buf, "alpha beta gamma", 16);
    place_caret(&e, 0, 0);
    feed(&e, "\033[1;6C"); /* Ctrl+Shift+Right */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("word to next start", "alpha ", got);
    feed(&e, "\033[1;6C");
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("extends by word", "alpha beta ", got);
    editor_free(&e);
}

void test_kbsel_word_left(void)
{
    Editor e;
    char got[64];
    editor_init(&e);
    buf_load_mem(&e.buf, "alpha beta gamma", 16);
    place_caret(&e, 0, 16);
    feed(&e, "\033[1;6D"); /* Ctrl+Shift+Left */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("word back", "gamma", got);
    feed(&e, "\033[1;6D");
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("word back again", "beta gamma", got);
    editor_free(&e);
}

void test_kbsel_home_end(void)
{
    Editor e;
    char got[64];
    editor_init(&e);
    buf_load_mem(&e.buf, "alpha beta gamma", 16);
    place_caret(&e, 0, 6);
    feed(&e, "\033[1;2H"); /* Shift+Home */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("to line start", "alpha ", got);
    place_caret(&e, 0, 6);
    feed(&e, "\033[1;2F"); /* Shift+End */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("to line end", "beta gamma", got);
    editor_free(&e);
}

void test_kbsel_doc_end(void)
{
    Editor e;
    char got[64];
    editor_init(&e);
    buf_load_mem(&e.buf, "one\ntwo\nthree", 13);
    place_caret(&e, 0, 0);
    feed(&e, "\033[1;6F"); /* Ctrl+Shift+End */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("to doc end", "one\ntwo\nthree", got);
    feed(&e, "\033[1;6H"); /* Ctrl+Shift+Home */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("back to doc home", "", got);
    editor_free(&e);
}

void test_kbsel_lines(void)
{
    Editor e;
    char got[64];
    editor_init(&e);
    buf_load_mem(&e.buf, "one\ntwo\nthree", 13);
    place_caret(&e, 0, 0);
    feed(&e, "\033[1;2B"); /* Shift+Down */
    feed(&e, "\033[1;2C"); /* Shift+Right */
    selected(&e, got, sizeof(got));
    ASSERT_STREQ("spans lines", "one\nt", got);
    editor_free(&e);
}

void test_kbsel_type_replaces(void)
{
    Editor e;
    size_t n = 0;
    char *s;
    editor_init(&e);
    buf_load_mem(&e.buf, "alpha beta gamma", 16);
    place_caret(&e, 0, 0);
    feed(&e, "\033[1;6C"); /* select "alpha " */
    feed(&e, "X");
    s = buf_to_string(&e.buf, &n);
    ASSERT_STREQ("typed over selection", "Xbeta gamma", s);
    free(s);
    ASSERT_EQ_INT("sel off after typing", 0, e.sel_on);
    editor_free(&e);
}
