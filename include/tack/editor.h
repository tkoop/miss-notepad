#ifndef TACK_EDITOR_H
#define TACK_EDITOR_H

#include "tack/buffer.h"
#include "tack/keys.h"
#include "tack/screen.h"

typedef struct {
    Buffer buf;
    size_t cx;
    size_t cy;
    size_t row_off;
    size_t col_off;
    int goal_col;
    int tabstop;
    int view_rows;
    int view_cols;
    int dirty;
    int quit;
    char *filename;
} Editor;

int editor_init(Editor *e);
void editor_free(Editor *e);
int editor_load_path(Editor *e, const char *path);
void editor_set_view(Editor *e, int rows, int cols);
void editor_move_left(Editor *e);
void editor_move_right(Editor *e);
void editor_move_up(Editor *e);
void editor_move_down(Editor *e);
void editor_move_home(Editor *e);
void editor_move_end(Editor *e);
void editor_move_page_up(Editor *e);
void editor_move_page_down(Editor *e);
void editor_move_doc_home(Editor *e);
void editor_move_doc_end(Editor *e);
void editor_move_word_left(Editor *e);
void editor_move_word_right(Editor *e);
void editor_scroll_into_view(Editor *e);
int editor_handle_event(Editor *e, const Event *ev);
void editor_render(const Editor *e, Screen *s);
int editor_cursor_col(const Editor *e);

#endif
