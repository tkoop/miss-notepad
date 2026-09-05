#include "missnotepad/search.h"

#include <stdlib.h>
#include <string.h>

static int find_in_line(const char *line, size_t len, size_t from,
                        const char *needle, size_t nlen, size_t *at)
{
    size_t i;
    if (nlen == 0 || from > len) {
        return 0;
    }
    for (i = from; i + nlen <= len; i++) {
        if (memcmp(line + i, needle, nlen) == 0) {
            *at = i;
            return 1;
        }
    }
    return 0;
}

int search_find(const Buffer *b, const char *needle, size_t start_row,
                size_t start_col, size_t *out_row, size_t *out_col)
{
    size_t nlen;
    size_t row;
    size_t count;
    if (needle == NULL || needle[0] == '\0') {
        return 0;
    }
    nlen = strlen(needle);
    count = buf_line_count(b);
    if (start_row >= count) {
        start_row = 0;
        start_col = 0;
    }
    for (row = start_row; row < count; row++) {
        size_t len = 0;
        const char *line = buf_line(b, row, &len);
        size_t from = row == start_row ? start_col : 0;
        size_t at = 0;
        if (find_in_line(line, len, from, needle, nlen, &at)) {
            *out_row = row;
            *out_col = at;
            return 1;
        }
    }
    for (row = 0; row <= start_row && row < count; row++) {
        size_t len = 0;
        const char *line = buf_line(b, row, &len);
        size_t from = 0;
        size_t at = 0;
        size_t limit = row == start_row ? start_col : len;
        (void)limit;
        if (find_in_line(line, len, from, needle, nlen, &at)) {
            if (row < start_row || at < start_col) {
                *out_row = row;
                *out_col = at;
                return 1;
            }
        }
    }
    return 0;
}

int search_replace_all(Buffer *b, const char *needle, const char *repl,
                       int *count)
{
    size_t nlen;
    size_t rlen;
    size_t row;
    int n = 0;
    if (needle == NULL || needle[0] == '\0') {
        if (count) {
            *count = 0;
        }
        return 0;
    }
    nlen = strlen(needle);
    rlen = repl ? strlen(repl) : 0;
    for (row = 0; row < buf_line_count(b); row++) {
        size_t col = 0;
        for (;;) {
            size_t len = 0;
            const char *line = buf_line(b, row, &len);
            size_t at = 0;
            if (!find_in_line(line, len, col, needle, nlen, &at)) {
                break;
            }
            if (buf_delete(b, row, at, nlen) != 0) {
                return -1;
            }
            if (rlen > 0) {
                if (buf_insert(b, row, at, repl, rlen) != 0) {
                    return -1;
                }
            }
            col = at + rlen;
            n++;
        }
    }
    if (count) {
        *count = n;
    }
    return 0;
}
