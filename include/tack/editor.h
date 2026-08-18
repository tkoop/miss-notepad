#ifndef TACK_EDITOR_H
#define TACK_EDITOR_H

#include "tack/buffer.h"
#include "tack/keys.h"
#include "tack/screen.h"

#include <stdint.h>

typedef enum { EDIT_INSERT = 0, EDIT_DELETE = 1 } EditKind;

typedef struct {
    EditKind kind;
    size_t row;
    size_t col;
    char *text;
    size_t len;
} UndoItem;

typedef struct {
    UndoItem *items;
    size_t count;
    size_t cap;
    size_t index;
} UndoStack;

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
    char status_msg[160];
    UndoStack undo;
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
int editor_insert_text(Editor *e, const char *s, size_t n);
int editor_insert_char(Editor *e, uint32_t cp);
int editor_newline(Editor *e);
int editor_backspace(Editor *e);
int editor_delete_forward(Editor *e);
int editor_undo(Editor *e);
int editor_redo(Editor *e);
int editor_new(Editor *e);
int editor_save(Editor *e);
int editor_save_as(Editor *e, const char *path);
void editor_set_message(Editor *e, const char *msg);

#endif
