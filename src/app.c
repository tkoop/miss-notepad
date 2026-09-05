#include "missnotepad/app.h"

#include "missnotepad/version.h"

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
                 "%s/.config/missnotepad/config", home);
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
    keybind_state_init(&app->keys);
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
        if (app->editor.theme == THEME_VI) {
            app->keys.vi_mode = VI_NORMAL;
        }
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
        dialog_show_saveas(&app->dialog, app->editor.filename);
        app->focus = FOCUS_DIALOG;
        break;
    case ACT_OPEN:
        dialog_show_open(&app->dialog);
        app->focus = FOCUS_DIALOG;
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
        dialog_show_about(&app->dialog);
        app->focus = FOCUS_DIALOG;
        break;
    case ACT_HELP_KEYS:
        editor_set_message(&app->editor, keybind_help(app->editor.theme));
        break;
    case ACT_THEME_NOTEPAD:
        app->editor.theme = THEME_NOTEPAD;
        keybind_state_init(&app->keys);
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: Notepad");
        break;
    case ACT_THEME_NANO:
        app->editor.theme = THEME_NANO;
        keybind_state_init(&app->keys);
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: nano");
        break;
    case ACT_THEME_VI:
        app->editor.theme = THEME_VI;
        keybind_state_init(&app->keys);
        app->keys.vi_mode = VI_NORMAL;
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: vi (normal)");
        break;
    case ACT_THEME_EMACS:
        app->editor.theme = THEME_EMACS;
        keybind_state_init(&app->keys);
        save_user_settings(app);
        editor_set_message(&app->editor, "Key theme: Emacs");
        break;
    case ACT_MOVE_LEFT:
        editor_move_left(&app->editor);
        break;
    case ACT_MOVE_RIGHT:
        editor_move_right(&app->editor);
        break;
    case ACT_MOVE_UP:
        editor_move_up(&app->editor);
        break;
    case ACT_MOVE_DOWN:
        editor_move_down(&app->editor);
        break;
    case ACT_MOVE_HOME:
        editor_move_home(&app->editor);
        break;
    case ACT_MOVE_END:
        editor_move_end(&app->editor);
        break;
    case ACT_KILL_LINE:
        editor_kill_line(&app->editor);
        break;
    case ACT_VI_COLON:
        dialog_show_vicmd(&app->dialog);
        app->focus = FOCUS_DIALOG;
        break;
    case ACT_FIND:
        dialog_show_find(&app->dialog, NULL);
        app->focus = FOCUS_DIALOG;
        break;
    case ACT_REPLACE:
        dialog_show_replace(&app->dialog, NULL);
        app->focus = FOCUS_DIALOG;
        break;
    case ACT_CUT:
        editor_cut(&app->editor);
        break;
    case ACT_COPY:
        editor_copy(&app->editor);
        break;
    case ACT_PASTE:
        editor_paste(&app->editor);
        break;
    case ACT_SELECT_ALL:
        editor_select_all(&app->editor);
        break;
    case ACT_FIND_NEXT:
        editor_find_next(&app->editor);
        break;
    case ACT_NONE:
    default:
        break;
    }
    return app->editor.quit;
}

static void finish_dialog(App *app)
{
    if (app->dialog.result == DLG_OK) {
        if (app->dialog.kind == DLG_OPEN) {
            if (editor_load_path(&app->editor, app->dialog.field) != 0) {
                editor_set_message(&app->editor, "Cannot open file");
            }
        } else if (app->dialog.kind == DLG_SAVEAS) {
            editor_save_as(&app->editor, app->dialog.field);
        } else if (app->dialog.kind == DLG_FIND) {
            memcpy(app->editor.find_text, app->dialog.field,
                   sizeof(app->editor.find_text) - 1);
            app->editor.find_text[sizeof(app->editor.find_text) - 1] = '\0';
            editor_find_next(&app->editor);
        } else if (app->dialog.kind == DLG_REPLACE) {
            editor_replace_all(&app->editor, app->dialog.field,
                               app->dialog.field2);
        } else if (app->dialog.kind == DLG_VICMD) {
            if (strcmp(app->dialog.field, "w") == 0 ||
                strcmp(app->dialog.field, "wq") == 0) {
                editor_save(&app->editor);
            }
            if (strcmp(app->dialog.field, "q") == 0 ||
                strcmp(app->dialog.field, "q!") == 0 ||
                strcmp(app->dialog.field, "wq") == 0) {
                app->editor.quit = 1;
            }
            if (strcmp(app->dialog.field, "w") != 0 &&
                strcmp(app->dialog.field, "q") != 0 &&
                strcmp(app->dialog.field, "q!") != 0 &&
                strcmp(app->dialog.field, "wq") != 0) {
                editor_set_message(&app->editor, "vi: use :w :q :wq :q!");
            }
        }
    }
    dialog_close(&app->dialog);
    app->focus = FOCUS_EDIT;
}

static int handle_mouse(App *app, const Event *ev)
{
    int btn = ev->mbtn;
    int wheel = (btn & 64) != 0;
    if (!ev->mdown && !wheel) {
        app->dragging = 0;
        return app->editor.quit;
    }
    if (app->dialog.visible) {
        dialog_handle_event(&app->dialog, ev);
        if (!app->dialog.visible) {
            finish_dialog(app);
        }
        return app->editor.quit;
    }
    if (wheel) {
        if ((btn & 1) == 0) {
            editor_move_up(&app->editor);
        } else {
            editor_move_down(&app->editor);
        }
        return app->editor.quit;
    }
    if ((btn & 3) != 0) {
        return app->editor.quit;
    }
    /* Left click */
    if (app->menu.active) {
        int item = menubar_hit_item(&app->menu, 1, ev->my, ev->mx);
        int bar = menubar_hit_bar(&app->menu, ev->my, ev->mx);
        if (item >= 0) {
            Action act = app->menu.menus[app->menu.open].items[item].action;
            menubar_close(&app->menu);
            app->focus = FOCUS_EDIT;
            return app_dispatch(app, act);
        }
        if (bar >= 0) {
            menubar_open(&app->menu, bar);
            app->focus = FOCUS_MENU;
            return app->editor.quit;
        }
        menubar_close(&app->menu);
        app->focus = FOCUS_EDIT;
        return app->editor.quit;
    }
    {
        int bar = menubar_hit_bar(&app->menu, ev->my, ev->mx);
        if (bar >= 0) {
            menubar_open(&app->menu, bar);
            app->focus = FOCUS_MENU;
            return app->editor.quit;
        }
    }
    if (ev->my > 0) {
        int gutter = editor_gutter_width(&app->editor);
        int extend = ((btn & 32) != 0) || app->dragging;
        if (!extend) {
            editor_sel_clear(&app->editor);
            app->editor.sy = app->editor.cy;
            app->editor.sx = app->editor.cx;
        }
        editor_click(&app->editor, ev->my - 1, ev->mx - gutter);
        if (!extend) {
            app->editor.sy = app->editor.cy;
            app->editor.sx = app->editor.cx;
            app->dragging = 1;
        } else {
            app->editor.sel_on = 1;
        }
    }
    return app->editor.quit;
}

int app_handle_event(App *app, const Event *ev)
{
    Action act;
    if (ev == NULL) {
        return app->editor.quit;
    }
    if (ev->kind == EV_MOUSE) {
        return handle_mouse(app, ev);
    }
    if (app->dialog.visible && ev->kind == EV_KEY) {
        dialog_handle_event(&app->dialog, ev);
        if (!app->dialog.visible) {
            finish_dialog(app);
        }
        return app->editor.quit;
    }
    if (ev->kind == EV_KEY && ev->key == KEY_CHAR &&
        ev->mods == MOD_CTRL && ev->ch == 'q') {
        return app_dispatch(app, ACT_EXIT);
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
        KeyCmd cmd = keybind_map(app->editor.theme, ev, &app->keys);
        if (cmd.kind == CMD_ACTION) {
            return app_dispatch(app, cmd.action);
        }
        if (cmd.kind == CMD_CONSUME) {
            return app->editor.quit;
        }
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
    if (app->dialog.visible) {
        dialog_render(&app->dialog, s);
    } else if (app->editor.theme == THEME_VI && s->rows > 0 && s->cols > 16) {
        const char *mode =
            app->keys.vi_mode == VI_INSERT ? "-- INSERT --" : "-- NORMAL --";
        screen_puts(s, s->rows - 1, s->cols - 14, mode, STYLE_STATUS);
    }
}
