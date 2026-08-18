#ifndef TACK_APP_H
#define TACK_APP_H

#include "tack/action.h"
#include "tack/dialog.h"
#include "tack/editor.h"
#include "tack/menu.h"
#include "tack/settings.h"

typedef enum { FOCUS_EDIT = 0, FOCUS_MENU = 1, FOCUS_DIALOG = 2 } AppFocus;

typedef struct {
    Editor editor;
    MenuBar menu;
    Dialog dialog;
    Settings settings;
    AppFocus focus;
    int dragging;
    char config_path[512];
} App;

int app_init(App *app, const char *filename);
void app_load_config(App *app, const char *path);
void app_free(App *app);
int app_handle_event(App *app, const Event *ev);
void app_render(App *app, Screen *s);
int app_dispatch(App *app, Action act);

#endif
