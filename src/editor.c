#include "tack/editor.h"

#include "tack/fileio.h"
#include "tack/utf8.h"
#include "tack/version.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_word(unsigned char c)
{
    return (c >= 128) || isalnum(c) || c == '_';
}

int editor_init(Editor *e)
{
    memset(e, 0, sizeof(*e));
    e->tabstop = TACK_TABSTOP_DEFAULT;
    e->view_rows = 24;
    e->view_cols = 80;
    e->filename = NULL;
    return buf_init(&e->buf);
}

static void undo_item_free(UndoItem *it)
{
    free(it->text);
    it->text = NULL;
    it->len = 0;
}

static void undo_clear_from(UndoStack *u, size_t from)
{
    size_t i;
    for (i = from; i < u->count; i++) {
        undo_item_free(&u->items[i]);
    }
    u->count = from;
    if (u->index > u->count) {
        u->index = u->count;
    }
}

static void undo_free(UndoStack *u)
{
    undo_clear_from(u, 0);
    free(u->items);
    u->items = NULL;
    u->cap = 0;
}

void editor_free(Editor *e)
{
    buf_free(&e->buf);
    undo_free(&e->undo);
    free(e->filename);
    e->filename = NULL;
}

static int set_filename(Editor *e, const char *path)
{
    char *copy;
    if (path == NULL) {
        free(e->filename);
        e->filename = NULL;
        return 0;
    }
    copy = malloc(strlen(path) + 1);
    if (copy == NULL) {
        return -1;
    }
    memcpy(copy, path, strlen(path) + 1);
    free(e->filename);
    e->filename = copy;
    return 0;
}

int editor_load_path(Editor *e, const char *path)
{
    char *data = NULL;
    size_t len = 0;
    int rc;

    if (set_filename(e, path) != 0) {
        return -1;
    }
    rc = file_read_all(path, &data, &len);
    if (rc == 1) {
        if (buf_clear(&e->buf) != 0) {
            return -1;
        }
        e->cx = 0;
        e->cy = 0;
        e->dirty = 0;
        return 0;
    }
    if (rc != 0) {
        return -1;
    }
    if (buf_load_mem(&e->buf, data, len) != 0) {
        free(data);
        return -1;
    }
    free(data);
    e->cx = 0;
    e->cy = 0;
    e->row_off = 0;
    e->col_off = 0;
    e->goal_col = 0;
    e->dirty = 0;
    return 0;
}

void editor_set_view(Editor *e, int rows, int cols)
{
    e->view_rows = rows > 0 ? rows : 1;
    e->view_cols = cols > 0 ? cols : 1;
    editor_scroll_into_view(e);
}

int editor_cursor_col(const Editor *e)
{
    size_t len = 0;
    const char *line = buf_line(&e->buf, e->cy, &len);
    return (int)utf8_col_of(line, len, e->cx, e->tabstop);
}

static void remember_goal(Editor *e)
{
    e->goal_col = editor_cursor_col(e);
}

static void apply_goal(Editor *e)
{
    size_t len = 0;
    const char *line = buf_line(&e->buf, e->cy, &len);
    e->cx = utf8_byte_at_col(line, len, (size_t)e->goal_col, e->tabstop);
}

void editor_scroll_into_view(Editor *e)
{
    int col = editor_cursor_col(e);
    int rows = e->view_rows;
    int cols = e->view_cols;

    if (e->cy < e->row_off) {
        e->row_off = e->cy;
    }
    if (rows > 0 && e->cy >= e->row_off + (size_t)rows) {
        e->row_off = e->cy - (size_t)rows + 1;
    }
    if (col < (int)e->col_off) {
        e->col_off = (size_t)col;
    }
    if (cols > 0 && col >= (int)e->col_off + cols) {
        e->col_off = (size_t)(col - cols + 1);
    }
}

void editor_move_left(Editor *e)
{
    if (e->cx > 0) {
        size_t len = 0;
        const char *line = buf_line(&e->buf, e->cy, &len);
        e->cx = utf8_prev(line, len, e->cx);
    } else if (e->cy > 0) {
        e->cy--;
        e->cx = buf_line_len(&e->buf, e->cy);
    }
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_right(Editor *e)
{
    size_t len = buf_line_len(&e->buf, e->cy);
    if (e->cx < len) {
        const char *line = buf_line(&e->buf, e->cy, NULL);
        e->cx = utf8_next(line, len, e->cx);
    } else if (e->cy + 1 < buf_line_count(&e->buf)) {
        e->cy++;
        e->cx = 0;
    }
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_up(Editor *e)
{
    if (e->cy > 0) {
        e->cy--;
        apply_goal(e);
        editor_scroll_into_view(e);
    }
}

void editor_move_down(Editor *e)
{
    if (e->cy + 1 < buf_line_count(&e->buf)) {
        e->cy++;
        apply_goal(e);
        editor_scroll_into_view(e);
    }
}

void editor_move_home(Editor *e)
{
    e->cx = 0;
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_end(Editor *e)
{
    e->cx = buf_line_len(&e->buf, e->cy);
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_page_up(Editor *e)
{
    size_t step = e->view_rows > 1 ? (size_t)e->view_rows - 1 : 1;
    if (e->cy > step) {
        e->cy -= step;
    } else {
        e->cy = 0;
    }
    apply_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_page_down(Editor *e)
{
    size_t step = e->view_rows > 1 ? (size_t)e->view_rows - 1 : 1;
    size_t last = buf_line_count(&e->buf) - 1;
    if (e->cy + step < last) {
        e->cy += step;
    } else {
        e->cy = last;
    }
    apply_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_doc_home(Editor *e)
{
    e->cy = 0;
    e->cx = 0;
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_doc_end(Editor *e)
{
    e->cy = buf_line_count(&e->buf) - 1;
    e->cx = buf_line_len(&e->buf, e->cy);
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_word_left(Editor *e)
{
    size_t len;
    const char *line;
    if (e->cx == 0) {
        editor_move_left(e);
        return;
    }
    line = buf_line(&e->buf, e->cy, &len);
    while (e->cx > 0) {
        size_t p = utf8_prev(line, len, e->cx);
        if (is_word((unsigned char)line[p])) {
            break;
        }
        e->cx = p;
    }
    while (e->cx > 0) {
        size_t p = utf8_prev(line, len, e->cx);
        if (!is_word((unsigned char)line[p])) {
            break;
        }
        e->cx = p;
    }
    remember_goal(e);
    editor_scroll_into_view(e);
}

void editor_move_word_right(Editor *e)
{
    size_t len;
    const char *line = buf_line(&e->buf, e->cy, &len);
    if (e->cx >= len) {
        editor_move_right(e);
        return;
    }
    while (e->cx < len && is_word((unsigned char)line[e->cx])) {
        e->cx = utf8_next(line, len, e->cx);
    }
    while (e->cx < len && !is_word((unsigned char)line[e->cx])) {
        e->cx = utf8_next(line, len, e->cx);
    }
    remember_goal(e);
    editor_scroll_into_view(e);
}

static int undo_reserve(UndoStack *u, size_t need)
{
    UndoItem *p;
    size_t cap;
    if (need <= u->cap) {
        return 0;
    }
    cap = u->cap == 0 ? 32 : u->cap;
    while (cap < need) {
        cap *= 2;
    }
    p = realloc(u->items, cap * sizeof(UndoItem));
    if (p == NULL) {
        return -1;
    }
    u->items = p;
    u->cap = cap;
    return 0;
}

static int undo_push(Editor *e, EditKind kind, size_t row, size_t col,
                     const char *text, size_t len)
{
    UndoItem *it;
    undo_clear_from(&e->undo, e->undo.index);
    if (undo_reserve(&e->undo, e->undo.count + 1) != 0) {
        return -1;
    }
    it = &e->undo.items[e->undo.count];
    it->kind = kind;
    it->row = row;
    it->col = col;
    it->len = len;
    it->text = malloc(len + 1);
    if (it->text == NULL) {
        return -1;
    }
    if (len > 0 && text != NULL) {
        memcpy(it->text, text, len);
    }
    it->text[len] = '\0';
    e->undo.count++;
    e->undo.index = e->undo.count;
    return 0;
}

static int apply_insert(Editor *e, size_t row, size_t col, const char *s,
                        size_t n, int record)
{
    size_t er, ec;
    if (buf_insert(&e->buf, row, col, s, n) != 0) {
        return -1;
    }
    buf_pos_after(row, col, s, n, &er, &ec);
    e->cy = er;
    e->cx = ec;
    e->dirty = 1;
    remember_goal(e);
    editor_scroll_into_view(e);
    if (record) {
        return undo_push(e, EDIT_INSERT, row, col, s, n);
    }
    return 0;
}

static int apply_delete_span(Editor *e, size_t r1, size_t c1, size_t r2,
                             size_t c2, int record)
{
    size_t n = 0;
    char *text;
    if (r1 == r2 && c1 == c2) {
        return 0;
    }
    text = buf_copy_span(&e->buf, r1, c1, r2, c2, &n);
    if (text == NULL) {
        return -1;
    }
    if (buf_delete_span(&e->buf, r1, c1, r2, c2) != 0) {
        free(text);
        return -1;
    }
    if (r1 > r2 || (r1 == r2 && c1 > c2)) {
        e->cy = r2;
        e->cx = c2;
    } else {
        e->cy = r1;
        e->cx = c1;
    }
    e->dirty = 1;
    remember_goal(e);
    editor_scroll_into_view(e);
    if (record) {
        if (undo_push(e, EDIT_DELETE, e->cy, e->cx, text, n) != 0) {
            free(text);
            return -1;
        }
    }
    free(text);
    return 0;
}

int editor_insert_text(Editor *e, const char *s, size_t n)
{
    if (s == NULL || n == 0) {
        return 0;
    }
    return apply_insert(e, e->cy, e->cx, s, n, 1);
}

int editor_insert_char(Editor *e, uint32_t cp)
{
    char u[4];
    int n = utf8_encode(cp, u);
    if (n <= 0) {
        return -1;
    }
    return editor_insert_text(e, u, (size_t)n);
}

int editor_newline(Editor *e)
{
    return editor_insert_text(e, "\n", 1);
}

int editor_backspace(Editor *e)
{
    if (e->cx == 0 && e->cy == 0) {
        return 0;
    }
    if (e->cx == 0) {
        size_t prev = e->cy - 1;
        size_t col = buf_line_len(&e->buf, prev);
        return apply_delete_span(e, prev, col, e->cy, 0, 1);
    } else {
        size_t len = 0;
        const char *line = buf_line(&e->buf, e->cy, &len);
        size_t prev = utf8_prev(line, len, e->cx);
        return apply_delete_span(e, e->cy, prev, e->cy, e->cx, 1);
    }
}

int editor_delete_forward(Editor *e)
{
    size_t len = buf_line_len(&e->buf, e->cy);
    if (e->cx < len) {
        const char *line = buf_line(&e->buf, e->cy, NULL);
        size_t next = utf8_next(line, len, e->cx);
        return apply_delete_span(e, e->cy, e->cx, e->cy, next, 1);
    }
    if (e->cy + 1 < buf_line_count(&e->buf)) {
        return apply_delete_span(e, e->cy, e->cx, e->cy + 1, 0, 1);
    }
    return 0;
}

int editor_undo(Editor *e)
{
    UndoItem *it;
    if (e->undo.index == 0) {
        return 0;
    }
    e->undo.index--;
    it = &e->undo.items[e->undo.index];
    if (it->kind == EDIT_INSERT) {
        size_t er, ec;
        buf_pos_after(it->row, it->col, it->text, it->len, &er, &ec);
        if (apply_delete_span(e, it->row, it->col, er, ec, 0) != 0) {
            return -1;
        }
    } else {
        if (apply_insert(e, it->row, it->col, it->text, it->len, 0) != 0) {
            return -1;
        }
    }
    if (e->undo.index == 0) {
        e->dirty = 0;
    }
    return 0;
}

int editor_redo(Editor *e)
{
    UndoItem *it;
    if (e->undo.index >= e->undo.count) {
        return 0;
    }
    it = &e->undo.items[e->undo.index];
    if (it->kind == EDIT_INSERT) {
        if (apply_insert(e, it->row, it->col, it->text, it->len, 0) != 0) {
            return -1;
        }
    } else {
        size_t er, ec;
        buf_pos_after(it->row, it->col, it->text, it->len, &er, &ec);
        if (apply_delete_span(e, it->row, it->col, er, ec, 0) != 0) {
            return -1;
        }
    }
    e->undo.index++;
    return 0;
}

int editor_handle_event(Editor *e, const Event *ev)
{
    if (ev == NULL || ev->kind != EV_KEY) {
        return e->quit;
    }
    if (ev->key == KEY_CHAR && ev->mods == MOD_CTRL && ev->ch == 'q') {
        e->quit = 1;
        return 1;
    }
    if (ev->key == KEY_CHAR && ev->mods == MOD_CTRL && ev->ch == 'z') {
        editor_undo(e);
        return e->quit;
    }
    if (ev->key == KEY_CHAR && ev->mods == MOD_CTRL && ev->ch == 'y') {
        editor_redo(e);
        return e->quit;
    }
    switch (ev->key) {
    case KEY_LEFT:
        if (ev->mods & MOD_CTRL) {
            editor_move_word_left(e);
        } else {
            editor_move_left(e);
        }
        break;
    case KEY_RIGHT:
        if (ev->mods & MOD_CTRL) {
            editor_move_word_right(e);
        } else {
            editor_move_right(e);
        }
        break;
    case KEY_UP:
        editor_move_up(e);
        break;
    case KEY_DOWN:
        editor_move_down(e);
        break;
    case KEY_HOME:
        if (ev->mods & MOD_CTRL) {
            editor_move_doc_home(e);
        } else {
            editor_move_home(e);
        }
        break;
    case KEY_END:
        if (ev->mods & MOD_CTRL) {
            editor_move_doc_end(e);
        } else {
            editor_move_end(e);
        }
        break;
    case KEY_PGUP:
        editor_move_page_up(e);
        break;
    case KEY_PGDN:
        editor_move_page_down(e);
        break;
    case KEY_ENTER:
        editor_newline(e);
        break;
    case KEY_BACKSPACE:
        editor_backspace(e);
        break;
    case KEY_DELETE:
        editor_delete_forward(e);
        break;
    case KEY_TAB:
        editor_insert_char(e, (uint32_t)'\t');
        break;
    case KEY_CHAR:
        if (ev->mods == 0 && ev->ch >= 32) {
            editor_insert_char(e, ev->ch);
        }
        break;
    default:
        break;
    }
    return e->quit;
}

static void render_text(const Editor *e, Screen *s, int y0, int x0, int h, int w)
{
    int r;
    for (r = 0; r < h; r++) {
        size_t row = e->row_off + (size_t)r;
        size_t len = 0;
        const char *line;
        size_t i;
        int x = 0;
        int skip = (int)e->col_off;

        screen_fill(s, y0 + r, x0, w, 1, (uint32_t)' ', STYLE_NORMAL);
        if (row >= buf_line_count(&e->buf)) {
            continue;
        }
        line = buf_line(&e->buf, row, &len);
        i = 0;
        while (i < len && x < w) {
            uint32_t cp = 0;
            size_t n = 1;
            int cw;
            if (utf8_decode(line, len, i, &cp, &n) != 0) {
                cp = (unsigned char)line[i];
                n = 1;
            }
            if (cp == '\t') {
                int ts = e->tabstop > 0 ? e->tabstop : 4;
                int display = (int)utf8_col_of(line, len, i, e->tabstop);
                cw = ts - (display % ts);
            } else {
                cw = utf8_codepoint_width(cp);
                if (cw <= 0) {
                    cw = 1;
                }
            }
            if (skip > 0) {
                if (skip >= cw) {
                    skip -= cw;
                    i += n;
                    continue;
                }
                cw -= skip;
                skip = 0;
                cp = (uint32_t)' ';
            }
            if (cp == '\t') {
                int k;
                for (k = 0; k < cw && x < w; k++) {
                    screen_put(s, y0 + r, x0 + x, (uint32_t)' ', STYLE_NORMAL);
                    x++;
                }
            } else {
                if (x + cw > w) {
                    break;
                }
                screen_put(s, y0 + r, x0 + x, cp, STYLE_NORMAL);
                x += cw;
            }
            i += n;
        }
    }
}

void editor_render(const Editor *e, Screen *s)
{
    char title[256];
    char status[256];
    const char *name = e->filename ? e->filename : "untitled";
    int text_h;
    int text_y;
    int cur_x;
    int cur_y;

    if (s->rows < 1 || s->cols < 1) {
        return;
    }

    snprintf(title, sizeof(title), " %s %s — %s%s", TACK_NAME, TACK_VERSION_STRING,
             name, e->dirty ? " *" : "");
    screen_fill(s, 0, 0, s->cols, 1, (uint32_t)' ', STYLE_TITLE);
    screen_puts(s, 0, 0, title, STYLE_TITLE);

    snprintf(status, sizeof(status), " Ln %d, Col %d    Ctrl+Z undo  Ctrl+Q quit",
             (int)e->cy + 1, editor_cursor_col(e) + 1);
    screen_fill(s, s->rows - 1, 0, s->cols, 1, (uint32_t)' ', STYLE_STATUS);
    screen_puts(s, s->rows - 1, 0, status, STYLE_STATUS);

    text_y = s->rows > 2 ? 1 : 0;
    text_h = s->rows > 2 ? s->rows - 2 : (s->rows > 1 ? 1 : 1);
    if (s->rows == 1) {
        text_h = 0;
    }
    if (text_h > 0) {
        render_text(e, s, text_y, 0, text_h, s->cols);
    }

    cur_y = text_y + (int)(e->cy - e->row_off);
    cur_x = editor_cursor_col(e) - (int)e->col_off;
    if (cur_x < 0) {
        cur_x = 0;
    }
    if (cur_y >= text_y && cur_y < text_y + text_h && cur_x < s->cols) {
        screen_cursor(s, cur_y, cur_x, 1);
    } else {
        screen_cursor(s, 0, 0, 0);
    }
}
