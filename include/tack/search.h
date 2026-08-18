#ifndef TACK_SEARCH_H
#define TACK_SEARCH_H

#include "tack/buffer.h"

#include <stddef.h>

int search_find(const Buffer *b, const char *needle, size_t start_row,
                size_t start_col, size_t *out_row, size_t *out_col);
int search_replace_all(Buffer *b, const char *needle, const char *repl,
                       int *count);

#endif
