#ifndef TACK_MENU_H
#define TACK_MENU_H

#include "tack/action.h"
#include "tack/keys.h"
#include "tack/screen.h"
#include "tack/settings.h"

#define TACK_MENU_MAX 8
#define TACK_ITEM_MAX 16

typedef struct {
    const char *label;
    const char *accel;
    char hot;
    Action action;
    int separator;
    int checkable;
} MenuItem;

typedef struct {
    const char *label;
    char hot;
    MenuItem items[TACK_ITEM_MAX];
    int count;
    int x;
    int w;
} Menu;

typedef struct {
    Menu menus[TACK_MENU_MAX];
    int count;
    int open;
    int highlight;
    int active;
} MenuBar;

void menubar_init_notepad(MenuBar *m);
void menubar_layout(MenuBar *m);
void menubar_close(MenuBar *m);
int menubar_open(MenuBar *m, int index);
int menubar_open_hot(MenuBar *m, char hot);
int menubar_hit_bar(const MenuBar *m, int y, int x);
int menubar_hit_item(const MenuBar *m, int drop_y, int y, int x);
Action menubar_handle_event(MenuBar *m, const Event *ev);
void menubar_render(const MenuBar *m, Screen *s, int y, int cols,
                    const char *right);
void menubar_sync_checks(MenuBar *m, int line_numbers, int word_wrap,
                         KeyTheme theme);

#endif
