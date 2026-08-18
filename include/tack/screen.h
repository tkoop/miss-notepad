#ifndef TACK_SCREEN_H
#define TACK_SCREEN_H

#include <stdint.h>

#define CELL_CONT 0xFFFFFFFFu

enum {
    STYLE_NORMAL = 0,
    STYLE_INVERSE,
    STYLE_TITLE,
    STYLE_STATUS,
    STYLE_GUTTER,
    STYLE_MENU,
    STYLE_MENU_HOT,
    STYLE_MENU_SEL,
    STYLE_DIALOG,
    STYLE_DIALOG_TITLE,
    STYLE_BUTTON,
    STYLE_BUTTON_FOCUS,
    STYLE_FIELD,
    STYLE_FIELD_FOCUS,
    STYLE_SELECT
};

typedef struct {
    uint32_t ch;
    unsigned char style;
} Cell;

typedef struct {
    int rows;
    int cols;
    Cell *cells;
    int cx;
    int cy;
    int show_cursor;
} Screen;

void screen_init(Screen *s);
void screen_free(Screen *s);
int screen_resize(Screen *s, int rows, int cols);
void screen_clear(Screen *s, unsigned char style);
void screen_put(Screen *s, int y, int x, uint32_t ch, unsigned char style);
int screen_puts(Screen *s, int y, int x, const char *utf8, unsigned char style);
void screen_fill(Screen *s, int y, int x, int w, int h, uint32_t ch,
                 unsigned char style);
uint32_t screen_get(const Screen *s, int y, int x);
unsigned char screen_get_style(const Screen *s, int y, int x);
void screen_cursor(Screen *s, int y, int x, int visible);

#endif
