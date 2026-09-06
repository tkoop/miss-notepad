#include "missnotepad/app.h"
#include "missnotepad/dialog.h"
#include "missnotepad/keys.h"
#include "test.h"

#include <stdio.h>
#include <unistd.h>

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

static Event mouse(int x, int y, int down)
{
    Event ev;
    ev.kind = EV_MOUSE;
    ev.key = KEY_CHAR;
    ev.ch = 0;
    ev.mods = 0;
    ev.mx = x;
    ev.my = y;
    ev.mbtn = 0;
    ev.mdown = down;
    return ev;
}

void test_mouse_parse(void)
{
    const char *s = "\033[<0;10;5M";
    Event ev;
    int n = event_parse((const unsigned char *)s, strlen(s), 0, &ev);
    ASSERT_TRUE("consumed", n > 0);
    ASSERT_EQ_INT("mouse", EV_MOUSE, ev.kind);
    ASSERT_EQ_INT("x", 9, ev.mx);
    ASSERT_EQ_INT("y", 4, ev.my);
    ASSERT_EQ_INT("down", 1, ev.mdown);
}

void test_dialog_type_and_ok(void)
{
    Dialog d;
    Event ev;
    dialog_show_open(&d);
    ASSERT_TRUE("visible", d.visible);
    ev = key(KEY_CHAR, 'a', 0);
    dialog_handle_event(&d, &ev);
    ev = key(KEY_CHAR, 'b', 0);
    dialog_handle_event(&d, &ev);
    ASSERT_STREQ("field", "ab", d.field);
    ev = key(KEY_ENTER, 0, 0);
    dialog_handle_event(&d, &ev);
    ASSERT_EQ_INT("ok", DLG_OK, d.result);
    ASSERT_TRUE("closed", !d.visible);
}

void test_dialog_esc_cancel(void)
{
    Dialog d;
    Event ev = key(KEY_ESC, 0, 0);
    dialog_show_about(&d);
    dialog_handle_event(&d, &ev);
    ASSERT_EQ_INT("cancel", DLG_CANCEL, d.result);
}

void test_dialog_render(void)
{
    Dialog d;
    Screen s;
    dialog_show_about(&d);
    screen_init(&s);
    screen_resize(&s, 20, 60);
    dialog_render(&d, &s);
    ASSERT_TRUE("title drawn", screen_get(&s, d.y, d.x + 2) != (uint32_t)' ');
    screen_free(&s);
}

void test_app_open_dialog_saveas(void)
{
    App app;
    char path[] = "/tmp/missnotepad-dlg-XXXXXX";
    int fd = mkstemp(path);
    Event ev;
    ASSERT_TRUE("mkstemp", fd >= 0);
    if (fd < 0) {
        return;
    }
    close(fd);
    unlink(path);

    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    app_dispatch(&app, ACT_SAVE_AS);
    ASSERT_TRUE("dialog", app.dialog.visible);
    {
        size_t i;
        for (i = 0; path[i]; i++) {
            ev = key(KEY_CHAR, (uint32_t)(unsigned char)path[i], 0);
            app_handle_event(&app, &ev);
        }
    }
    ev = key(KEY_ENTER, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("closed", !app.dialog.visible);
    ASSERT_STREQ("saved name", path, app.editor.filename);
    app_free(&app);
    unlink(path);
}

void test_app_save_untitled(void)
{
    App app;
    char path[] = "/tmp/missnotepad-sav-XXXXXX";
    char path2[] = "/tmp/missnotepad-sav2-XXXXXX";
    int fd = mkstemp(path);
    int fd2 = mkstemp(path2);
    Event ev;
    ASSERT_TRUE("mkstemp", fd >= 0 && fd2 >= 0);
    if (fd < 0 || fd2 < 0) {
        return;
    }
    close(fd);
    close(fd2);
    unlink(path);
    unlink(path2);

    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    ASSERT_TRUE("no name", app.editor.filename == NULL);

    /* Save on an untitled document asks for a file name first. */
    app_dispatch(&app, ACT_SAVE);
    ASSERT_TRUE("saveas dialog", app.dialog.visible);
    ASSERT_EQ_INT("kind", DLG_SAVEAS, (int)app.dialog.kind);
    ASSERT_EQ_INT("field empty", 0, (int)app.dialog.field[0]);
    ASSERT_EQ_INT("focus", FOCUS_DIALOG, (int)app.focus);

    {
        size_t i;
        for (i = 0; path[i]; i++) {
            ev = key(KEY_CHAR, (uint32_t)(unsigned char)path[i], 0);
            app_handle_event(&app, &ev);
        }
    }
    ev = key(KEY_ENTER, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("closed", !app.dialog.visible);
    ASSERT_STREQ("name kept", path, app.editor.filename);
    ASSERT_EQ_INT("saved", 0, (int)app.editor.dirty);

    /* With a file name, Save does not ask again. */
    app_dispatch(&app, ACT_SAVE);
    ASSERT_TRUE("no dialog", !app.dialog.visible);
    ASSERT_EQ_INT("focus back", FOCUS_EDIT, (int)app.focus);

    /* vi :wq on an untitled document also asks for a name, then quits
       once the save completes. */
    editor_new(&app.editor);
    ASSERT_TRUE("untitled again", app.editor.filename == NULL);
    app_dispatch(&app, ACT_VI_COLON);
    ASSERT_TRUE("vicmd dialog", app.dialog.visible);
    ev = key(KEY_CHAR, 'w', 0);
    app_handle_event(&app, &ev);
    ev = key(KEY_CHAR, 'q', 0);
    app_handle_event(&app, &ev);
    ev = key(KEY_ENTER, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("saveas asked", app.dialog.visible);
    ASSERT_EQ_INT("saveas kind", DLG_SAVEAS, (int)app.dialog.kind);
    {
        size_t i;
        for (i = 0; path2[i]; i++) {
            ev = key(KEY_CHAR, (uint32_t)(unsigned char)path2[i], 0);
            app_handle_event(&app, &ev);
        }
    }
    ev = key(KEY_ENTER, 0, 0);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("saveas closed", !app.dialog.visible);
    ASSERT_STREQ("name2 kept", path2, app.editor.filename);
    ASSERT_EQ_INT("quit after save", 1, app.editor.quit);

    app_free(&app);
    unlink(path);
    unlink(path2);
}

void test_editor_click(void)
{
    Editor e;
    editor_init(&e);
    buf_load_mem(&e.buf, "abcdef\nxyz", 10);
    editor_set_view(&e, 10, 40);
    editor_click(&e, 0, 3);
    ASSERT_EQ_INT("y", 0, (int)e.cy);
    ASSERT_EQ_INT("x", 3, (int)e.cx);
    editor_click(&e, 1, 1);
    ASSERT_EQ_INT("y2", 1, (int)e.cy);
    ASSERT_EQ_INT("x2", 1, (int)e.cx);
    editor_free(&e);
}

void test_app_mouse_menu(void)
{
    App app;
    Event ev;
    Screen s;
    ASSERT_EQ_INT("init", 0, app_init(&app, NULL));
    screen_init(&s);
    screen_resize(&s, 16, 60);
    editor_set_view(&app.editor, 14, 60);
    app_render(&app, &s);
    ev = mouse(app.menu.menus[0].x + 1, 0, 1);
    app_handle_event(&app, &ev);
    ASSERT_TRUE("menu open", app.menu.active);
    ASSERT_EQ_INT("File", 0, app.menu.open);
    screen_free(&s);
    app_free(&app);
}
