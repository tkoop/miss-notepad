#include "missnotepad/app.h"
#include "missnotepad/menu.h"
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

void test_menubar_init(void)
{
    MenuBar m;
    menubar_init_notepad(&m);
    ASSERT_EQ_INT("5 menus", 5, m.count);
    ASSERT_STREQ("File", "File", m.menus[0].label);
    ASSERT_STREQ("Edit", "Edit", m.menus[1].label);
    ASSERT_TRUE("File has items", m.menus[0].count > 3);
    ASSERT_EQ_INT("closed", -1, m.open);
}

void test_menubar_hotkeys(void)
{
    MenuBar m;
    menubar_init_notepad(&m);
    ASSERT_TRUE("open F", menubar_open_hot(&m, 'f'));
    ASSERT_EQ_INT("File", 0, m.open);
    ASSERT_TRUE("active", m.active);
    menubar_close(&m);
    ASSERT_EQ_INT("closed", -1, m.open);
}

void test_menubar_hit_bar(void)
{
    MenuBar m;
    menubar_init_notepad(&m);
    ASSERT_EQ_INT("file hit", 0, menubar_hit_bar(&m, 0, m.menus[0].x + 1));
    ASSERT_EQ_INT("miss row", -1, menubar_hit_bar(&m, 2, 2));
}

void test_menubar_navigate_and_activate(void)
{
    MenuBar m;
    Event ev;
    Action act;
    menubar_init_notepad(&m);
    ev = key(KEY_F10, 0, 0);
    ASSERT_EQ_INT("f10 none", ACT_NONE, menubar_handle_event(&m, &ev));
    ASSERT_TRUE("opened", m.active);
    /* File: New, Open, Save, Save As, sep, Exit — Exit is last */
    ev = key(KEY_CHAR, 'x', 0);
    act = menubar_handle_event(&m, &ev);
    ASSERT_EQ_INT("exit", ACT_EXIT, act);
    ASSERT_TRUE("closed after", !m.active);
}

void test_app_menu_exit(void)
{
    App app;
    Event ev;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    ev = key(KEY_F10, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_EQ_INT("menu focus", FOCUS_MENU, app.focus);
    ev = key(KEY_CHAR, 'x', 0);
    app_handle_event(&app, &ev);
    ASSERT_EQ_INT("quit", 1, app.editor.quit);
    app_free(&app);
}

void test_app_menu_ctrl_q_exit(void)
{
    App app;
    Event ev;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    ev = key(KEY_F10, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("menu open", app.menu.active);
    ev = key(KEY_CHAR, 'q', MOD_CTRL);
    app_handle_event(&app, &ev);
    ASSERT_EQ_INT("quit", 1, app.editor.quit);
    app_free(&app);
}

void test_app_line_numbers_action(void)
{
    App app;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    ASSERT_EQ_INT("on", 1, app.editor.show_linenum);
    app.config_path[0] = '\0'; /* do not write the user's real config */
    app_dispatch(&app, ACT_LINE_NUMBERS);
    ASSERT_EQ_INT("off", 0, app.editor.show_linenum);
    app_free(&app);
}

void test_app_render_menu(void)
{
    App app;
    Screen s;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    screen_init(&s);
    screen_resize(&s, 12, 60);
    editor_set_view(&app.editor, 10, 60);
    app_render(&app, &s);
    ASSERT_EQ_INT("menu style", STYLE_MENU, screen_get_style(&s, 0, 1));
    ASSERT_EQ_INT("F of File", (int)'F', (int)screen_get(&s, 0, 1));
    menubar_open(&app.menu, 0);
    app_render(&app, &s);
    ASSERT_TRUE("dropdown", screen_get(&s, 1, app.menu.menus[0].x) != 0);
    screen_free(&s);
    app_free(&app);
}

void test_app_alt_opens_help(void)
{
    App app;
    Event ev;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    ev = key(KEY_CHAR, 'h', MOD_ALT);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("open", app.menu.active);
    ASSERT_EQ_INT("Help", 4, app.menu.open);
    app_free(&app);
}

static const MenuItem *find_item(const MenuBar *m, Action act)
{
    int i, j;
    for (i = 0; i < m->count; i++) {
        for (j = 0; j < m->menus[i].count; j++) {
            if (m->menus[i].items[j].action == act) {
                return &m->menus[i].items[j];
            }
        }
    }
    return NULL;
}

void test_menu_theme_accels(void)
{
    MenuBar m;
    const MenuItem *it;
    menubar_init_notepad(&m);

    /* Notepad theme (default) */
    menubar_sync_checks(&m, 1, 1, THEME_NOTEPAD);
    it = find_item(&m, ACT_SAVE);
    ASSERT_STREQ("notepad save accel", "Ctrl+S", it->accel);
    it = find_item(&m, ACT_EXIT);
    ASSERT_STREQ("notepad exit accel", "Ctrl+Q", it->accel);

    /* nano theme */
    menubar_sync_checks(&m, 1, 1, THEME_NANO);
    it = find_item(&m, ACT_SAVE);
    ASSERT_STREQ("nano save accel", "^S", it->accel);
    it = find_item(&m, ACT_SAVE_AS);
    ASSERT_STREQ("nano save-as accel", "^O", it->accel);
    it = find_item(&m, ACT_EXIT);
    ASSERT_STREQ("nano exit accel", "^X", it->accel);
    it = find_item(&m, ACT_FIND);
    ASSERT_STREQ("nano find accel", "^W", it->accel);
    it = find_item(&m, ACT_PASTE);
    ASSERT_STREQ("nano paste accel", "^U", it->accel);
    it = find_item(&m, ACT_COPY);
    ASSERT_NULL("nano copy has no hint", it->accel);

    /* vi theme */
    menubar_sync_checks(&m, 1, 1, THEME_VI);
    it = find_item(&m, ACT_SAVE);
    ASSERT_STREQ("vi save accel", ":w", it->accel);
    it = find_item(&m, ACT_EXIT);
    ASSERT_STREQ("vi exit accel", ":q", it->accel);
    it = find_item(&m, ACT_DELETE);
    ASSERT_STREQ("vi delete accel", "x", it->accel);
    it = find_item(&m, ACT_PASTE);
    ASSERT_STREQ("vi paste accel", "p", it->accel);

    /* Emacs theme */
    menubar_sync_checks(&m, 1, 1, THEME_EMACS);
    it = find_item(&m, ACT_SAVE);
    ASSERT_STREQ("emacs save accel", "C-x C-s", it->accel);
    it = find_item(&m, ACT_SAVE_AS);
    ASSERT_STREQ("emacs save as accel", "C-x C-w", it->accel);
    it = find_item(&m, ACT_EXIT);
    ASSERT_STREQ("emacs exit accel", "C-x C-c", it->accel);
}

void test_app_theme_switch_updates_accels(void)
{
    App app;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    app.config_path[0] = '\0'; /* do not write the user's real config */
    ASSERT_STREQ("starts notepad", "Ctrl+S",
                 find_item(&app.menu, ACT_SAVE)->accel);
    app_dispatch(&app, ACT_THEME_NANO);
    ASSERT_STREQ("nano save after dispatch", "^S",
                 find_item(&app.menu, ACT_SAVE)->accel);
    ASSERT_STREQ("nano save-as after dispatch", "^O",
                 find_item(&app.menu, ACT_SAVE_AS)->accel);
    app_dispatch(&app, ACT_THEME_EMACS);
    ASSERT_STREQ("emacs after dispatch", "C-x C-s",
                 find_item(&app.menu, ACT_SAVE)->accel);
    app_dispatch(&app, ACT_THEME_NOTEPAD);
    ASSERT_STREQ("back to notepad", "Ctrl+S",
                 find_item(&app.menu, ACT_SAVE)->accel);
    app_free(&app);
}
