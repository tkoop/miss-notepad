#ifndef MISSNOTEPAD_APP_H
#define MISSNOTEPAD_APP_H

#include "missnotepad/action.h"
#include "missnotepad/dialog.h"
#include "missnotepad/editor.h"
#include "missnotepad/keybind.h"
#include "missnotepad/menu.h"
#include "missnotepad/settings.h"

typedef enum { FOCUS_EDIT = 0, FOCUS_MENU = 1, FOCUS_DIALOG = 2 } AppFocus;

typedef struct {
    Editor editor;
    MenuBar menu;
    Dialog dialog;
    Settings settings;
    AppFocus focus;
    int dragging;
    int quit_after_save; /* vi :wq on untitled: quit after the name is picked */
    KeybindState keys;
    char config_path[512];
} App;

int app_init(App *app, const char *filename);
void app_load_config(App *app, const char *path);
void app_apply_cli_options(App *app, int keys_set, KeyTheme keys_theme,
                           int wrap_set, int wrap);
void app_free(App *app);
int app_handle_event(App *app, const Event *ev);
void app_render(App *app, Screen *s);
int app_dispatch(App *app, Action act);

#endif
