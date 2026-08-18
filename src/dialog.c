#include "tack/dialog.h"

#include "tack/utf8.h"
#include "tack/version.h"

#include <stdio.h>
#include <string.h>

void dialog_close(Dialog *d)
{
    d->visible = 0;
    d->kind = DLG_NONE;
    d->result = DLG_PENDING;
}

static void setup(Dialog *d, DialogKind kind, const char *title,
                  const char *prompt, int has_field, int has_field2)
{
    memset(d, 0, sizeof(*d));
    d->kind = kind;
    d->visible = 1;
    d->has_field = has_field;
    d->has_field2 = has_field2;
    d->focus = has_field ? 0 : 2;
    d->result = DLG_PENDING;
    snprintf(d->title, sizeof(d->title), "%s", title);
    snprintf(d->prompt, sizeof(d->prompt), "%s", prompt ? prompt : "");
}

void dialog_show_open(Dialog *d)
{
    setup(d, DLG_OPEN, "Open", "File name:", 1, 0);
}

void dialog_show_saveas(Dialog *d, const char *preset)
{
    setup(d, DLG_SAVEAS, "Save As", "File name:", 1, 0);
    if (preset) {
        snprintf(d->field, sizeof(d->field), "%s", preset);
        d->cursor = (int)strlen(d->field);
    }
}

void dialog_show_about(Dialog *d)
{
    char prompt[80];
    snprintf(prompt, sizeof(prompt), "%s %s — Notepad for the terminal",
             TACK_NAME, TACK_VERSION_STRING);
    setup(d, DLG_ABOUT, "About Tack", prompt, 0, 0);
}

void dialog_show_find(Dialog *d, const char *preset)
{
    setup(d, DLG_FIND, "Find", "Find what:", 1, 0);
    if (preset) {
        snprintf(d->field, sizeof(d->field), "%s", preset);
        d->cursor = (int)strlen(d->field);
    }
}

void dialog_show_replace(Dialog *d, const char *preset)
{
    setup(d, DLG_REPLACE, "Replace", "Find what:", 1, 1);
    if (preset) {
        snprintf(d->field, sizeof(d->field), "%s", preset);
        d->cursor = (int)strlen(d->field);
    }
}

void dialog_show_confirm(Dialog *d, const char *title, const char *prompt)
{
    setup(d, DLG_CONFIRM, title ? title : "Confirm", prompt, 0, 0);
}

static void insert_char(Dialog *d, uint32_t cp)
{
    char u[4];
    int n;
    int len;
    char *field = d->focus == 1 ? d->field2 : d->field;
    size_t cap = d->focus == 1 ? sizeof(d->field2) : sizeof(d->field);
    if (d->focus > 1) {
        return;
    }
    n = utf8_encode(cp, u);
    if (n <= 0) {
        return;
    }
    len = (int)strlen(field);
    if (d->cursor < 0) {
        d->cursor = 0;
    }
    if (d->cursor > len) {
        d->cursor = len;
    }
    if ((size_t)len + (size_t)n >= cap) {
        return;
    }
    memmove(field + d->cursor + n, field + d->cursor, (size_t)(len - d->cursor) + 1);
    memcpy(field + d->cursor, u, (size_t)n);
    d->cursor += n;
}

static void field_backspace(Dialog *d)
{
    char *field = d->focus == 1 ? d->field2 : d->field;
    size_t len;
    size_t prev;
    if (d->focus > 1 || d->cursor <= 0) {
        return;
    }
    len = strlen(field);
    prev = utf8_prev(field, len, (size_t)d->cursor);
    memmove(field + prev, field + d->cursor, len - (size_t)d->cursor + 1);
    d->cursor = (int)prev;
}

int dialog_handle_event(Dialog *d, const Event *ev)
{
    if (!d->visible || ev == NULL) {
        return 0;
    }
    if (ev->kind == EV_MOUSE && ev->mdown && (ev->mbtn & 3) == 0 &&
        (ev->mbtn & 64) == 0) {
        int hit = dialog_hit(d, ev->my, ev->mx);
        if (hit == 0) {
            d->result = DLG_CANCEL;
            d->visible = 0;
            return 1;
        }
        if (hit == 1) {
            d->focus = 0;
        } else if (hit == 2) {
            d->focus = 1;
        } else if (hit == 3) {
            d->result = DLG_OK;
            d->visible = 0;
            return 1;
        } else if (hit == 4) {
            d->result = DLG_CANCEL;
            d->visible = 0;
            return 1;
        }
        return 1;
    }
    if (ev->kind != EV_KEY) {
        return 0;
    }
    if (ev->key == KEY_ESC) {
        d->result = DLG_CANCEL;
        d->visible = 0;
        return 1;
    }
    if (ev->key == KEY_ENTER) {
        if (d->focus == 3) {
            d->result = DLG_CANCEL;
        } else {
            d->result = DLG_OK;
        }
        d->visible = 0;
        return 1;
    }
    if (ev->key == KEY_TAB) {
        int max = d->has_field2 ? 3 : (d->has_field ? 3 : 3);
        d->focus++;
        if (!d->has_field && d->focus < 2) {
            d->focus = 2;
        }
        if (!d->has_field2 && d->focus == 1) {
            d->focus = 2;
        }
        if (d->focus > max) {
            d->focus = d->has_field ? 0 : 2;
        }
        return 1;
    }
    if (d->focus <= 1 && d->has_field) {
        if (ev->key == KEY_BACKSPACE) {
            field_backspace(d);
            return 1;
        }
        if (ev->key == KEY_LEFT && d->cursor > 0) {
            char *field = d->focus == 1 ? d->field2 : d->field;
            d->cursor = (int)utf8_prev(field, strlen(field), (size_t)d->cursor);
            return 1;
        }
        if (ev->key == KEY_RIGHT) {
            char *field = d->focus == 1 ? d->field2 : d->field;
            size_t len = strlen(field);
            if ((size_t)d->cursor < len) {
                d->cursor = (int)utf8_next(field, len, (size_t)d->cursor);
            }
            return 1;
        }
        if (ev->key == KEY_CHAR && ev->mods == 0 && ev->ch >= 32) {
            insert_char(d, ev->ch);
            return 1;
        }
    }
    return 1;
}

static void layout(Dialog *d, int rows, int cols)
{
    d->w = cols < 50 ? cols - 2 : 50;
    if (d->w < 24) {
        d->w = cols > 4 ? cols - 2 : cols;
    }
    d->h = d->has_field2 ? 10 : (d->has_field ? 8 : 6);
    d->x = (cols - d->w) / 2;
    d->y = (rows - d->h) / 2;
    if (d->x < 0) {
        d->x = 0;
    }
    if (d->y < 1) {
        d->y = 1;
    }
}

int dialog_hit(const Dialog *d, int y, int x)
{
    int okx, canx, by;
    if (!d->visible) {
        return 0;
    }
    if (y < d->y || y >= d->y + d->h || x < d->x || x >= d->x + d->w) {
        return 0;
    }
    if (d->has_field && y == d->y + 3) {
        return 1;
    }
    if (d->has_field2 && y == d->y + 5) {
        return 2;
    }
    by = d->y + d->h - 2;
    okx = d->x + d->w - 22;
    canx = d->x + d->w - 12;
    if (y == by && x >= okx && x < okx + 8) {
        return 3;
    }
    if (y == by && x >= canx && x < canx + 10) {
        return 4;
    }
    return 5; /* inside dialog, no control */
}

void dialog_render(Dialog *d, Screen *s)
{
    int r;
    int okx, canx, by;
    if (!d->visible) {
        return;
    }
    layout(d, s->rows, s->cols);
    screen_fill(s, d->y, d->x, d->w, d->h, (uint32_t)' ', STYLE_DIALOG);
    for (r = 0; r < d->w; r++) {
        screen_put(s, d->y, d->x + r, (uint32_t)'-', STYLE_DIALOG_TITLE);
        screen_put(s, d->y + d->h - 1, d->x + r, (uint32_t)'-', STYLE_DIALOG);
    }
    for (r = 0; r < d->h; r++) {
        screen_put(s, d->y + r, d->x, (uint32_t)'|', STYLE_DIALOG);
        screen_put(s, d->y + r, d->x + d->w - 1, (uint32_t)'|', STYLE_DIALOG);
    }
    screen_fill(s, d->y, d->x, d->w, 1, (uint32_t)' ', STYLE_DIALOG_TITLE);
    screen_puts(s, d->y, d->x + 2, d->title, STYLE_DIALOG_TITLE);
    screen_puts(s, d->y + 2, d->x + 2, d->prompt, STYLE_DIALOG);
    if (d->has_field) {
        unsigned char st = d->focus == 0 ? STYLE_FIELD_FOCUS : STYLE_FIELD;
        screen_fill(s, d->y + 3, d->x + 2, d->w - 4, 1, (uint32_t)' ', st);
        screen_puts(s, d->y + 3, d->x + 2, d->field, st);
    }
    if (d->has_field2) {
        unsigned char st = d->focus == 1 ? STYLE_FIELD_FOCUS : STYLE_FIELD;
        screen_puts(s, d->y + 4, d->x + 2, "Replace with:", STYLE_DIALOG);
        screen_fill(s, d->y + 5, d->x + 2, d->w - 4, 1, (uint32_t)' ', st);
        screen_puts(s, d->y + 5, d->x + 2, d->field2, st);
    }
    by = d->y + d->h - 2;
    okx = d->x + d->w - 22;
    canx = d->x + d->w - 12;
    screen_puts(s, by, okx, "[ OK ]",
                d->focus == 2 ? STYLE_BUTTON_FOCUS : STYLE_BUTTON);
    if (d->kind != DLG_ABOUT) {
        screen_puts(s, by, canx, "[ Cancel ]",
                    d->focus == 3 ? STYLE_BUTTON_FOCUS : STYLE_BUTTON);
    }
    if (d->has_field && d->focus <= 1) {
        int fx = d->x + 2 + d->cursor;
        int fy = d->focus == 1 ? d->y + 5 : d->y + 3;
        screen_cursor(s, fy, fx, 1);
    }
}
