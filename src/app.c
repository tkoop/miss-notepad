#include "tack/app.h"

#include "tack/version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void ensure_config_dir(const char *path)
{
    char dir[512];
    char *slash;
    snprintf(dir, sizeof(dir), "%s", path);
    slash = strrchr(dir, '/');
    if (slash == NULL) {
        return;
    }
    *slash = '\0';
    mkdir(dir, 0755);
}

static void load_user_settings(App *app)
{
    const char *home = getenv("HOME");
    settings_defaults(&app->settings);
    app->config_path[0] = '\0';
    if (home != NULL) {
        snprintf(app->config_path, sizeof(app->config_path),
                 "%s/.config/tack/config", home);
        settings_load(&app->settings, app->config_path);
    }
    editor_apply_settings(&app->editor, &app->settings);
}

static void save_user_settings(App *app)
{
    app->settings.show_linenum = app->editor.show_linenum;
    app->settings.word_wrap = app->editor.word_wrap;
    app->settings.tabstop = app->editor.tabstop;
    app->settings.theme = app->editor.theme;
    if (app->config_path[0] != '\0') {
        ensure_config_dir(app->config_path);
        settings_save(&app->settings, app->config_path);
    }
    menubar_sync_checks(&app->menu, app->editor.show_linenum,
                        app->editor.word_wrap, app->editor.theme);
}

int app_init(App *app, const char *filename)
{
    memset(app, 0, sizeof(*app));
    if (editor_init(&app->editor) != 0) {
        return -1;
    }
    settings_defaults(&app->settings);
    editor_apply_settings(&app->editor, &app->settings);
    menubar_init_notepad(&app->menu);
    menubar_sync_checks(&app->menu, app->editor.show_linenum,
                        app->editor.word_wrap, app->editor.theme);
    app->focus = FOCUS_EDIT;
    if (filename != NULL) {
        if (editor_load_path(&app->editor, filename) != 0) {
            return -1;
        }
    }
    return 0;
}

void app_load_config(App *app, const char *path)
{
    if (path == NULL || path[0] == '\0') {
        load_user_settings(app);
    } else {
        snprintf(app->config_path, sizeof(app->config_path), "%s", path);
        settings_load(&app->settings, path);
        editor_apply_settings(&app->editor, &app->settings);
    }
    menubar_sync_checks(&app->menu, app->editor.show_linenum,
                        app->editor.word_wrap, app->editor.theme);
}

void app_free(App *app)
{
    editor_free(&app->editor);
}

int app_dispatch(App *app, Action act)
{
    switch (act) {
    case ACT_NEW:
        editor_new(&app->editor);
        break;
    case ACT_SAVE:
        editor_save(&app->editor);
        break;
    case ACT_SAVE_AS:
        editor_set_message(&app->editor, "Save As: pop-up dialogs come next");
        break;
    case ACT_OPEN:
        editor_set_message(&app->editor, "Open: pop-up dialogs come next");
        break;
    case ACT_EXIT:
        app->editor.quit = 1;
        break;
    case ACT_UNDO:
        editor_undo(&app->editor);
        break;
    case ACT_REDO:
        editor_redo(&app->editor);
        break;
    case ACT_DELETE:
        editor_delete_forward(&app->editor);
        break;
    case ACT_LINE_NUMBERS:
        editor_toggle_line_numbers(&app->editor);
        save_user_settings(app);
        editor_set_message(&app->editor, app->editor.show_linenum
                                             ? "Line numbers on"
                                             : "Line numbers off");
        break;
    case ACT_WORD_WRAP:
        app->editor.word_wrap = !app->editor.word_wrap;
        save_user_settings(app);
        editor_set_message(&app->editor, app->editor.word_wrap
                                             ? "Word wrap on"
                                             : "Word wrap off");
        break;
    case ACT_ABOUT:
        editor_set_message(&app->editor, "Tack " TACK_VERSION_STRING
                                         " — Notepad for the terminal");
        break;
    case ACT_HELP_KEYS:
        editor_set_message(&app->editor,
                           "Alt+letter opens menus. Ctrl+S save, Ctrl+Q quit.");
        break;
    case ACT_THEME_NOTEPAD:
        app->editor.theme = THEME_NOTEPAD;
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: Notepad");
        break;
    case ACT_THEME_NANO:
        app->editor.theme = THEME_NANO;
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: nano");
        break;
    case ACT_THEME_VI:
        app->editor.theme = THEME_VI;
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: vi");
        break;
    case ACT_THEME_EMACS:
        app->editor.theme = THEME_EMACS;
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: Emacs");
        break;
    case ACT_CUT:
    case ACT_COPY:
    case ACT_PASTE:
    case ACT_SELECT_ALL:
    case ACT_FIND:
    case ACT_FIND_NEXT:
    case ACT_REPLACE:
        editor_set_message(&app->editor,
                           "That command arrives in a later version");
        break;
    case ACT_NONE:
    default:
        break;
    }
    return app->editor.quit;
}

int app_handle_event(App *app, const Event *ev)
{
    Action act;
    if (ev == NULL) {
        return app->editor.quit;
    }
    if (ev->kind == EV_KEY &&
        (app->menu.active || ev->key == KEY_F10 || (ev->mods & MOD_ALT))) {
        act = menubar_handle_event(&app->menu, ev);
        app->focus = app->menu.active ? FOCUS_MENU : FOCUS_EDIT;
        if (act != ACT_NONE) {
            return app_dispatch(app, act);
        }
        return app->editor.quit;
    }
    if (ev->kind == EV_KEY) {
        editor_handle_event(&app->editor, ev);
    }
    return app->editor.quit;
}

void app_render(App *app, Screen *s)
{
    char right[160];
    const char *name = app->editor.filename ? app->editor.filename : "untitled";
    editor_render(&app->editor, s);
    snprintf(right, sizeof(right), "%s%s", name, app->editor.dirty ? " *" : "");
    menubar_sync_checks(&app->menu, app->editor.show_linenum,
                        app->editor.word_wrap, app->editor.theme);
    menubar_render(&app->menu, s, 0, s->cols, right);
}
