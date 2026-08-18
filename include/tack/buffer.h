#ifndef TACK_BUFFER_H
#define TACK_BUFFER_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} BufLine;

typedef struct {
    BufLine *lines;
    size_t count;
    size_t cap;
} Buffer;

int buf_init(Buffer *b);
void buf_free(Buffer *b);
int buf_clear(Buffer *b);

size_t buf_line_count(const Buffer *b);
const char *buf_line(const Buffer *b, size_t row, size_t *len);
size_t buf_line_len(const Buffer *b, size_t row);

int buf_insert(Buffer *b, size_t row, size_t col, const char *s, size_t n);
int buf_insert_cstr(Buffer *b, size_t row, size_t col, const char *s);
int buf_delete(Buffer *b, size_t row, size_t col, size_t n);
int buf_split(Buffer *b, size_t row, size_t col);
int buf_join_next(Buffer *b, size_t row);

int buf_load_mem(Buffer *b, const char *data, size_t n);
char *buf_to_string(const Buffer *b, size_t *out_len);

#endif
