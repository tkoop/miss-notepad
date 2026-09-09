#include "missnotepad/app.h"
#include "missnotepad/keybind.h"
#include "test.h"

static Event key(KeyId id, uint32_t ch, int mods)
{
    Event ev;
    ev.kind = EV_KEY;
    ev.key = id;
    ev.ch = ch;
    ev.mods = mods;
    ev.mx = ev.my = ev.mbtn = ev.mdown = 0;
    return ev;
}

void test_keybind_notepad(void)
{
    KeybindState st;
    Event ev = key(KEY_CHAR, 's', MOD_CTRL);
    KeyCmd c;
    keybind_state_init(&st);
    c = keybind_map(THEME_NOTEPAD, &ev, &st);
    ASSERT_EQ_INT("save", ACT_SAVE, c.action);
    ev = key(KEY_CHAR, 'q', MOD_CTRL);
    c = keybind_map(THEME_NOTEPAD, &ev, &st);
    ASSERT_EQ_INT("quit", ACT_EXIT, c.action);
}

void test_keybind_nano(void)
{
    KeybindState st;
    Event ev = key(KEY_CHAR, 'x', MOD_CTRL);
    KeyCmd c;
    keybind_state_init(&st);
    c = keybind_map(THEME_NANO, &ev, &st);
    ASSERT_EQ_INT("nano exit", ACT_EXIT, c.action);
    ev = key(KEY_CHAR, 'o', MOD_CTRL);
    c = keybind_map(THEME_NANO, &ev, &st);
    ASSERT_EQ_INT("nano save", ACT_SAVE, c.action);
}

void test_keybind_emacs_prefix(void)
{
    KeybindState st;
    Event ev = key(KEY_CHAR, 'x', MOD_CTRL);
    KeyCmd c;
    keybind_state_init(&st);
    c = keybind_map(THEME_EMACS, &ev, &st);
    ASSERT_EQ_INT("prefix consume", CMD_CONSUME, c.kind);
    ev = key(KEY_CHAR, 's', MOD_CTRL);
    c = keybind_map(THEME_EMACS, &ev, &st);
    ASSERT_EQ_INT("C-x C-s save", ACT_SAVE, c.action);
    ev = key(KEY_CHAR, 'x', MOD_CTRL);
    keybind_map(THEME_EMACS, &ev, &st);
    ev = key(KEY_CHAR, 'c', MOD_CTRL);
    c = keybind_map(THEME_EMACS, &ev, &st);
    ASSERT_EQ_INT("C-x C-c quit", ACT_EXIT, c.action);
}

void test_keybind_vi_modes(void)
{
    KeybindState st;
    Event ev;
    KeyCmd c;
    keybind_state_init(&st);
    st.vi_mode = VI_NORMAL;
    ev = key(KEY_CHAR, 'i', 0);
    c = keybind_map(THEME_VI, &ev, &st);
    ASSERT_EQ_INT("insert mode", VI_INSERT, st.vi_mode);
    ev = key(KEY_ESC, 0, 0);
    c = keybind_map(THEME_VI, &ev, &st);
    ASSERT_EQ_INT("normal", VI_NORMAL, st.vi_mode);
    ev = key(KEY_CHAR, 'l', 0);
    c = keybind_map(THEME_VI, &ev, &st);
    ASSERT_EQ_INT("l right", ACT_MOVE_RIGHT, c.action);
    ev = key(KEY_CHAR, ':', 0);
    c = keybind_map(THEME_VI, &ev, &st);
    ASSERT_EQ_INT("colon", ACT_VI_COLON, c.action);
    (void)c;
}

void test_app_vi_colon_quit(void)
{
    App app;
    Event ev;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    app.editor.theme = THEME_VI;
    app.keys.vi_mode = VI_NORMAL;
    ev = key(KEY_CHAR, ':', 0);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("dialog", app.dialog.visible);
    ev = key(KEY_CHAR, 'q', 0);
    app_handle_event(&app, &ev);
    ev = key(KEY_ENTER, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_EQ_INT("quit", 1, app.editor.quit);
    app_free(&app);
}

void test_keybind_help_strings(void)
{
    ASSERT_TRUE("notepad", strstr(keybind_help(THEME_NOTEPAD), "Ctrl+S") != NULL);
    ASSERT_TRUE("nano", strstr(keybind_help(THEME_NANO), "^X") != NULL);
    ASSERT_TRUE("vi", strstr(keybind_help(THEME_VI), "hjkl") != NULL);
    ASSERT_TRUE("emacs", strstr(keybind_help(THEME_EMACS), "C-x C-s") != NULL);
}

void test_keybind_status_hints(void)
{
    ASSERT_TRUE("hint notepad", strstr(keybind_status_hint(THEME_NOTEPAD),
                                       "Ctrl+S") != NULL);
    ASSERT_TRUE("hint nano", strstr(keybind_status_hint(THEME_NANO),
                                    "^O save") != NULL);
    ASSERT_TRUE("hint vi", strstr(keybind_status_hint(THEME_VI),
                                  ":w save") != NULL);
    ASSERT_TRUE("hint emacs", strstr(keybind_status_hint(THEME_EMACS),
                                     "C-x C-s save") != NULL);
}

void test_status_bar_follows_theme(void)
{
    Editor e;
    Screen s;
    int y;
    int found = 0;
    int x;
    char line[256];
    size_t n = 0;

    editor_init(&e);
    screen_init(&s);
    screen_resize(&s, 8, 70);
    editor_set_view(&e, 6, 70);

    e.theme = THEME_NANO;
    editor_render(&e, &s);
    y = s.rows - 1;
    for (x = 0; x < s.cols && n + 1 < sizeof(line); x++) {
        line[n++] = (char)screen_get(&s, y, x);
    }
    line[n] = '\0';
    found = strstr(line, "^O save") != NULL &&
            strstr(line, "Ctrl+S") == NULL;
    ASSERT_TRUE("nano hint on status line", found);

    n = 0;
    e.theme = THEME_EMACS;
    editor_render(&e, &s);
    for (x = 0; x < s.cols && n + 1 < sizeof(line); x++) {
        line[n++] = (char)screen_get(&s, y, x);
    }
    line[n] = '\0';
    ASSERT_TRUE("emacs hint on status line",
                strstr(line, "C-x C-s save") != NULL);
    ASSERT_TRUE("no notepad hint in emacs",
                strstr(line, "Ctrl+S") == NULL);

    n = 0;
    e.theme = THEME_VI;
    editor_render(&e, &s);
    for (x = 0; x < s.cols && n + 1 < sizeof(line); x++) {
        line[n++] = (char)screen_get(&s, y, x);
    }
    line[n] = '\0';
    ASSERT_TRUE("vi hint on status line", strstr(line, ":w save") != NULL);

    n = 0;
    e.theme = THEME_NOTEPAD;
    editor_render(&e, &s);
    for (x = 0; x < s.cols && n + 1 < sizeof(line); x++) {
        line[n++] = (char)screen_get(&s, y, x);
    }
    line[n] = '\0';
    ASSERT_TRUE("notepad hint restored", strstr(line, "Ctrl+S save") != NULL);

    screen_free(&s);
    editor_free(&e);
}
