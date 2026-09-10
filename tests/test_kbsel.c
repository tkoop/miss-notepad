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

/* ---- App-level tests: the exact TUI path (event_parse -> app_handle_event
 * -> keybind_map -> editor) per theme. ---- */
#include "missnotepad/app.h"

static void appfeed(App *app, const char *bytes)
{
    Event ev;
    int used = event_parse((const unsigned char *)bytes, strlen(bytes), 0, &ev);
    if (used > 0) {
        app_handle_event(app, &ev);
    }
}

static const char *clip_of(App *app, char *out, size_t n)
{
    editor_copy(&app->editor);
    if (app->editor.clip != NULL) {
        snprintf(out, n, "%.*s", (int)app->editor.clip_len, app->editor.clip);
    } else {
        snprintf(out, n, "%s", "");
    }
    return out;
}

static void assert_sel(App *app, const char *name, const char *want)
{
    char got[64];
    char msg[160];
    clip_of(app, got, sizeof(got));
    snprintf(msg, sizeof(msg), "got=\"%s\" cy=%zu cx=%zu sel_on=%d", got,
             app->editor.cy, app->editor.cx, app->editor.sel_on);
    ASSERT_TRUE(name, strcmp(got, want) == 0);
    if (strcmp(got, want) != 0) {
        fprintf(stderr, "    detail: %s\n", msg);
    }
}

static void test_word_extend_in_theme(KeyTheme theme, int vi_insert_first)
{
    App app;
    char name[80];
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    app.config_path[0] = '\0'; /* do not write the user's real config */
    app.editor.theme = theme;
    app.keys.vi_mode = vi_insert_first ? VI_INSERT : VI_NORMAL;
    editor_set_view(&app.editor, 20, 80);
    editor_insert_text(&app.editor, "alpha beta gamma", 16);
    appfeed(&app, "\033[1;5H"); /* Ctrl+Home: doc start */

    appfeed(&app, "\033[1;6C"); /* Ctrl+Shift+Right */
    snprintf(name, sizeof(name), "theme %d word extend", (int)theme);
    assert_sel(&app, name, "alpha ");
    appfeed(&app, "\033[1;6C");
    snprintf(name, sizeof(name), "theme %d word extend again", (int)theme);
    assert_sel(&app, name, "alpha beta ");

    appfeed(&app, "\033[1;6D"); /* Ctrl+Shift+Left crosses back */
    snprintf(name, sizeof(name), "theme %d word shrink", (int)theme);
    assert_sel(&app, name, "alpha ");
    appfeed(&app, "\033[1;6D");
    snprintf(name, sizeof(name), "theme %d word collapse", (int)theme);
    assert_sel(&app, name, "");

    appfeed(&app, "\033[1;6C"); /* and extend anew */
    snprintf(name, sizeof(name), "theme %d word re-extend", (int)theme);
    assert_sel(&app, name, "alpha ");

    /* plain Ctrl+Right clears the selection and word-moves */
    appfeed(&app, "\033[1;5C");
    snprintf(name, sizeof(name), "theme %d plain ctrl clears", (int)theme);
    ASSERT_TRUE(name, app.editor.sel_on == 0);
    ASSERT_EQ_INT("theme word-moved caret", 11, (int)app.editor.cx);

    /* extending back from the new caret selects to the word start */
    appfeed(&app, "\033[1;6D");
    snprintf(name, sizeof(name), "theme %d extend back from new caret",
             (int)theme);
    assert_sel(&app, name, "beta ");
    app_free(&app);
}

void test_app_kbsel_notepad(void)
{
    test_word_extend_in_theme(THEME_NOTEPAD, 0);
}

void test_app_kbsel_nano(void)
{
    test_word_extend_in_theme(THEME_NANO, 0);
}

void test_app_kbsel_emacs(void)
{
    test_word_extend_in_theme(THEME_EMACS, 0);
}

void test_app_kbsel_vi_insert(void)
{
    test_word_extend_in_theme(THEME_VI, 1);
}

void test_app_kbsel_multiline(void)
{
    App app;
    char name[80];
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    app.config_path[0] = '\0';
    editor_set_view(&app.editor, 20, 80);
    editor_insert_text(&app.editor, "one\ntwo\nthree", 13);
    appfeed(&app, "\033[1;5H"); /* Ctrl+Home: doc start */

    appfeed(&app, "\033[1;2B"); /* Shift+Down */
    snprintf(name, sizeof(name), "ml down1");
    assert_sel(&app, name, "one\n");
    appfeed(&app, "\033[1;2B");
    snprintf(name, sizeof(name), "ml down2");
    assert_sel(&app, name, "one\ntwo\n");
    appfeed(&app, "\033[1;6C"); /* Ctrl+Shift+Right from (2,0): to end of "three" */
    snprintf(name, sizeof(name), "ml word extend over lines");
    assert_sel(&app, name, "one\ntwo\nthree");
    appfeed(&app, "\033[1;6D"); /* Ctrl+Shift+Left back to start of "three" */
    snprintf(name, sizeof(name), "ml word shrink");
    assert_sel(&app, name, "one\ntwo\n");
    app_free(&app);
}
