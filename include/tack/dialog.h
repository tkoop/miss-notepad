#ifndef TACK_DIALOG_H
#define TACK_DIALOG_H

#include "tack/keys.h"
#include "tack/screen.h"

typedef enum {
    DLG_NONE = 0,
    DLG_OPEN,
    DLG_SAVEAS,
    DLG_ABOUT,
    DLG_FIND,
    DLG_REPLACE,
    DLG_CONFIRM
} DialogKind;

typedef enum {
    DLG_PENDING = 0,
    DLG_OK = 1,
    DLG_CANCEL = -1
} DialogResult;

typedef struct {
    DialogKind kind;
    int visible;
    char title[80];
    char prompt[80];
    char field[512];
    char field2[512];
    int has_field;
    int has_field2;
    int cursor;
    int focus; /* 0 field, 1 field2, 2 ok, 3 cancel */
    int x, y, w, h;
    DialogResult result;
} Dialog;

void dialog_close(Dialog *d);
void dialog_show_open(Dialog *d);
void dialog_show_saveas(Dialog *d, const char *preset);
void dialog_show_about(Dialog *d);
void dialog_show_find(Dialog *d, const char *preset);
void dialog_show_replace(Dialog *d, const char *preset);
void dialog_show_confirm(Dialog *d, const char *title, const char *prompt);
int dialog_handle_event(Dialog *d, const Event *ev);
void dialog_render(Dialog *d, Screen *s);
int dialog_hit(const Dialog *d, int y, int x);

#endif
