#include "tack/buffer.h"

#include <stdlib.h>
#include <string.h>

static int line_reserve(BufLine *line, size_t need)
{
    char *p;
    size_t cap;

    if (need <= line->cap) {
        return 0;
    }
    cap = line->cap == 0 ? 16 : line->cap;
    while (cap < need) {
        cap *= 2;
    }
    p = realloc(line->data, cap);
    if (p == NULL) {
        return -1;
    }
    line->data = p;
    line->cap = cap;
    return 0;
}

static int buf_reserve(Buffer *b, size_t need)
{
    BufLine *p;
    size_t cap;

    if (need <= b->cap) {
        return 0;
    }
    cap = b->cap == 0 ? 8 : b->cap;
    while (cap < need) {
        cap *= 2;
    }
    p = realloc(b->lines, cap * sizeof(BufLine));
    if (p == NULL) {
        return -1;
    }
    b->lines = p;
    b->cap = cap;
    return 0;
}

static int line_init(BufLine *line)
{
    line->data = NULL;
    line->len = 0;
    line->cap = 0;
    return 0;
}

static void line_free(BufLine *line)
{
    free(line->data);
    line->data = NULL;
    line->len = 0;
    line->cap = 0;
}

int buf_init(Buffer *b)
{
    b->lines = NULL;
    b->count = 0;
    b->cap = 0;
    return buf_clear(b);
}

void buf_free(Buffer *b)
{
    size_t i;
    if (b->lines != NULL) {
        for (i = 0; i < b->count; i++) {
            line_free(&b->lines[i]);
        }
        free(b->lines);
    }
    b->lines = NULL;
    b->count = 0;
    b->cap = 0;
}

int buf_clear(Buffer *b)
{
    size_t i;
    for (i = 0; i < b->count; i++) {
        line_free(&b->lines[i]);
    }
    if (buf_reserve(b, 1) != 0) {
        return -1;
    }
    line_init(&b->lines[0]);
    b->count = 1;
    return 0;
}

size_t buf_line_count(const Buffer *b)
{
    return b->count;
}

const char *buf_line(const Buffer *b, size_t row, size_t *len)
{
    if (row >= b->count) {
        if (len) {
            *len = 0;
        }
        return "";
    }
    if (len) {
        *len = b->lines[row].len;
    }
    return b->lines[row].data ? b->lines[row].data : "";
}

size_t buf_line_len(const Buffer *b, size_t row)
{
    if (row >= b->count) {
        return 0;
    }
    return b->lines[row].len;
}

static int line_insert(BufLine *line, size_t col, const char *s, size_t n)
{
    if (col > line->len) {
        col = line->len;
    }
    if (line_reserve(line, line->len + n + 1) != 0) {
        return -1;
    }
    if (line->data == NULL) {
        return -1;
    }
    memmove(line->data + col + n, line->data + col, line->len - col);
    memcpy(line->data + col, s, n);
    line->len += n;
    line->data[line->len] = '\0';
    return 0;
}

static int line_delete(BufLine *line, size_t col, size_t n)
{
    if (col > line->len) {
        return 0;
    }
    if (col + n > line->len) {
        n = line->len - col;
    }
    if (n == 0 || line->data == NULL) {
        return 0;
    }
    memmove(line->data + col, line->data + col + n, line->len - col - n);
    line->len -= n;
    line->data[line->len] = '\0';
    return 0;
}

int buf_insert(Buffer *b, size_t row, size_t col, const char *s, size_t n)
{
    size_t i;
    size_t start;
    size_t pos;

    if (b->count == 0 && buf_clear(b) != 0) {
        return -1;
    }
    if (row >= b->count) {
        row = b->count - 1;
        col = b->lines[row].len;
    }
    if (col > b->lines[row].len) {
        col = b->lines[row].len;
    }
    if (s == NULL || n == 0) {
        return 0;
    }

    start = 0;
    pos = col;
    for (i = 0; i <= n; i++) {
        if (i == n || s[i] == '\n') {
            size_t chunk = i - start;
            /* Drop a preceding CR so CRLF becomes a single break. */
            if (chunk > 0 && s[start + chunk - 1] == '\r') {
                chunk--;
            }
            if (line_insert(&b->lines[row], pos, s + start, chunk) != 0) {
                return -1;
            }
            if (i == n) {
                return 0;
            }
            if (buf_split(b, row, pos + chunk) != 0) {
                return -1;
            }
            row++;
            pos = 0;
            start = i + 1;
        }
    }
    return 0;
}

int buf_insert_cstr(Buffer *b, size_t row, size_t col, const char *s)
{
    if (s == NULL) {
        return 0;
    }
    return buf_insert(b, row, col, s, strlen(s));
}

int buf_delete(Buffer *b, size_t row, size_t col, size_t n)
{
    if (row >= b->count) {
        return 0;
    }
    if (col > b->lines[row].len) {
        col = b->lines[row].len;
    }
    return line_delete(&b->lines[row], col, n);
}

int buf_split(Buffer *b, size_t row, size_t col)
{
    BufLine *src;
    BufLine extra;
    size_t rest;

    if (row >= b->count) {
        return -1;
    }
    src = &b->lines[row];
    if (col > src->len) {
        col = src->len;
    }
    if (buf_reserve(b, b->count + 1) != 0) {
        return -1;
    }
    src = &b->lines[row];
    rest = src->len - col;
    line_init(&extra);
    if (rest > 0) {
        if (line_reserve(&extra, rest + 1) != 0) {
            return -1;
        }
        memcpy(extra.data, src->data + col, rest);
        extra.len = rest;
        extra.data[rest] = '\0';
        src->len = col;
        if (src->data) {
            src->data[src->len] = '\0';
        }
    }
    memmove(&b->lines[row + 2], &b->lines[row + 1],
            (b->count - row - 1) * sizeof(BufLine));
    b->lines[row + 1] = extra;
    b->count++;
    return 0;
}

int buf_join_next(Buffer *b, size_t row)
{
    BufLine *a;
    BufLine *c;
    if (row + 1 >= b->count) {
        return 0;
    }
    a = &b->lines[row];
    c = &b->lines[row + 1];
    if (c->len > 0) {
        if (line_insert(a, a->len, c->data, c->len) != 0) {
            return -1;
        }
    }
    line_free(c);
    memmove(&b->lines[row + 1], &b->lines[row + 2],
            (b->count - row - 2) * sizeof(BufLine));
    b->count--;
    return 0;
}

int buf_load_mem(Buffer *b, const char *data, size_t n)
{
    if (buf_clear(b) != 0) {
        return -1;
    }
    if (data == NULL || n == 0) {
        return 0;
    }
    return buf_insert(b, 0, 0, data, n);
}

char *buf_to_string(const Buffer *b, size_t *out_len)
{
    size_t i;
    size_t total = 0;
    char *out;
    size_t pos = 0;

    if (b->count == 0) {
        out = malloc(1);
        if (out == NULL) {
            return NULL;
        }
        out[0] = '\0';
        if (out_len) {
            *out_len = 0;
        }
        return out;
    }
    for (i = 0; i < b->count; i++) {
        total += b->lines[i].len;
        if (i + 1 < b->count) {
            total += 1; /* newline */
        }
    }
    out = malloc(total + 1);
    if (out == NULL) {
        return NULL;
    }
    for (i = 0; i < b->count; i++) {
        if (b->lines[i].len > 0 && b->lines[i].data != NULL) {
            memcpy(out + pos, b->lines[i].data, b->lines[i].len);
            pos += b->lines[i].len;
        }
        if (i + 1 < b->count) {
            out[pos++] = '\n';
        }
    }
    out[pos] = '\0';
    if (out_len) {
        *out_len = pos;
    }
    return out;
}

static void normalize_span(const Buffer *b, size_t *r1, size_t *c1, size_t *r2,
                           size_t *c2)
{
    if (*r1 > *r2 || (*r1 == *r2 && *c1 > *c2)) {
        size_t tr = *r1, tc = *c1;
        *r1 = *r2;
        *c1 = *c2;
        *r2 = tr;
        *c2 = tc;
    }
    if (*r1 >= b->count) {
        *r1 = b->count ? b->count - 1 : 0;
    }
    if (*r2 >= b->count) {
        *r2 = b->count ? b->count - 1 : 0;
    }
    if (*c1 > b->lines[*r1].len) {
        *c1 = b->lines[*r1].len;
    }
    if (*c2 > b->lines[*r2].len) {
        *c2 = b->lines[*r2].len;
    }
}

void buf_pos_after(size_t row, size_t col, const char *s, size_t n, size_t *orow,
                   size_t *ocol)
{
    size_t i;
    *orow = row;
    *ocol = col;
    for (i = 0; i < n; i++) {
        if (s[i] == '\n') {
            (*orow)++;
            *ocol = 0;
        } else if (s[i] != '\r') {
            (*ocol)++;
        }
    }
}

char *buf_copy_span(const Buffer *b, size_t r1, size_t c1, size_t r2, size_t c2,
                    size_t *out_len)
{
    size_t row;
    size_t total = 0;
    char *out;
    size_t pos = 0;

    normalize_span(b, &r1, &c1, &r2, &c2);
    if (r1 == r2) {
        size_t n = c2 - c1;
        out = malloc(n + 1);
        if (out == NULL) {
            return NULL;
        }
        if (n > 0 && b->lines[r1].data != NULL) {
            memcpy(out, b->lines[r1].data + c1, n);
        }
        out[n] = '\0';
        if (out_len) {
            *out_len = n;
        }
        return out;
    }
    total += b->lines[r1].len - c1;
    total += 1;
    for (row = r1 + 1; row < r2; row++) {
        total += b->lines[row].len + 1;
    }
    total += c2;
    out = malloc(total + 1);
    if (out == NULL) {
        return NULL;
    }
    if (b->lines[r1].len > c1 && b->lines[r1].data != NULL) {
        memcpy(out + pos, b->lines[r1].data + c1, b->lines[r1].len - c1);
    }
    pos += b->lines[r1].len - c1;
    out[pos++] = '\n';
    for (row = r1 + 1; row < r2; row++) {
        if (b->lines[row].len > 0 && b->lines[row].data != NULL) {
            memcpy(out + pos, b->lines[row].data, b->lines[row].len);
        }
        pos += b->lines[row].len;
        out[pos++] = '\n';
    }
    if (c2 > 0 && b->lines[r2].data != NULL) {
        memcpy(out + pos, b->lines[r2].data, c2);
    }
    pos += c2;
    out[pos] = '\0';
    if (out_len) {
        *out_len = pos;
    }
    return out;
}

int buf_delete_span(Buffer *b, size_t r1, size_t c1, size_t r2, size_t c2)
{
    size_t row;

    normalize_span(b, &r1, &c1, &r2, &c2);
    if (r1 == r2) {
        return buf_delete(b, r1, c1, c2 - c1);
    }
    /* Keep prefix of first line + suffix of last line; drop middle lines. */
    if (buf_delete(b, r1, c1, b->lines[r1].len - c1) != 0) {
        return -1;
    }
    if (c2 > 0) {
        if (line_insert(&b->lines[r1], b->lines[r1].len, b->lines[r2].data + c2,
                        b->lines[r2].len - c2) != 0) {
            return -1;
        }
    } else if (b->lines[r2].len > 0) {
        if (line_insert(&b->lines[r1], b->lines[r1].len, b->lines[r2].data,
                        b->lines[r2].len) != 0) {
            return -1;
        }
    }
    for (row = r2; row > r1; row--) {
        line_free(&b->lines[row]);
    }
    memmove(&b->lines[r1 + 1], &b->lines[r2 + 1],
            (b->count - r2 - 1) * sizeof(BufLine));
    b->count -= (r2 - r1);
    return 0;
}
