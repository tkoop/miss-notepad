#include "tack/screen.h"

#include "tack/utf8.h"

#include <stdlib.h>
#include <string.h>

void screen_init(Screen *s)
{
    s->rows = 0;
    s->cols = 0;
    s->cells = NULL;
    s->cx = 0;
    s->cy = 0;
    s->show_cursor = 1;
}

void screen_free(Screen *s)
{
    free(s->cells);
    s->cells = NULL;
    s->rows = 0;
    s->cols = 0;
}

int screen_resize(Screen *s, int rows, int cols)
{
    Cell *p;
    size_t n;
    if (rows < 1) {
        rows = 1;
    }
    if (cols < 1) {
        cols = 1;
    }
    n = (size_t)rows * (size_t)cols;
    p = realloc(s->cells, n * sizeof(Cell));
    if (p == NULL) {
        return -1;
    }
    s->cells = p;
    s->rows = rows;
    s->cols = cols;
    screen_clear(s, STYLE_NORMAL);
    return 0;
}

void screen_clear(Screen *s, unsigned char style)
{
    int i;
    int n = s->rows * s->cols;
    for (i = 0; i < n; i++) {
        s->cells[i].ch = (uint32_t)' ';
        s->cells[i].style = style;
    }
}

static int in_bounds(const Screen *s, int y, int x)
{
    return y >= 0 && x >= 0 && y < s->rows && x < s->cols;
}

void screen_put(Screen *s, int y, int x, uint32_t ch, unsigned char style)
{
    int w;
    if (!in_bounds(s, y, x)) {
        return;
    }
    s->cells[y * s->cols + x].ch = ch;
    s->cells[y * s->cols + x].style = style;
    w = utf8_codepoint_width(ch);
    if (w >= 2 && x + 1 < s->cols) {
        s->cells[y * s->cols + x + 1].ch = CELL_CONT;
        s->cells[y * s->cols + x + 1].style = style;
    }
}

int screen_puts(Screen *s, int y, int x, const char *utf8, unsigned char style)
{
    size_t i = 0;
    size_t len;
    int start = x;
    if (utf8 == NULL || !in_bounds(s, y, 0)) {
        return 0;
    }
    len = strlen(utf8);
    while (utf8[i] != '\0' && x < s->cols) {
        uint32_t cp = 0;
        size_t n = 1;
        int w;
        if (utf8_decode(utf8, len, i, &cp, &n) != 0) {
            cp = (unsigned char)utf8[i];
            n = 1;
        }
        w = utf8_codepoint_width(cp);
        if (w <= 0) {
            w = 1;
        }
        if (x + w > s->cols) {
            break;
        }
        screen_put(s, y, x, cp, style);
        x += w;
        i += n;
    }
    return x - start;
}

void screen_fill(Screen *s, int y, int x, int w, int h, uint32_t ch,
                 unsigned char style)
{
    int yy, xx;
    for (yy = 0; yy < h; yy++) {
        for (xx = 0; xx < w; xx++) {
            screen_put(s, y + yy, x + xx, ch, style);
        }
    }
}

uint32_t screen_get(const Screen *s, int y, int x)
{
    if (!in_bounds(s, y, x)) {
        return 0;
    }
    return s->cells[y * s->cols + x].ch;
}

unsigned char screen_get_style(const Screen *s, int y, int x)
{
    if (!in_bounds(s, y, x)) {
        return 0;
    }
    return s->cells[y * s->cols + x].style;
}

void screen_cursor(Screen *s, int y, int x, int visible)
{
    s->cy = y;
    s->cx = x;
    s->show_cursor = visible;
}
