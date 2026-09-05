#ifndef MISSNOTEPAD_UTF8_H
#define MISSNOTEPAD_UTF8_H

#include <stddef.h>
#include <stdint.h>

#define MISSNOTEPAD_TABSTOP_DEFAULT 4

size_t utf8_next(const char *s, size_t len, size_t i);
size_t utf8_prev(const char *s, size_t len, size_t i);
int utf8_decode(const char *s, size_t len, size_t i, uint32_t *cp, size_t *nbytes);
int utf8_codepoint_width(uint32_t cp);
size_t utf8_display_width(const char *s, size_t len, size_t bytes, int tabstop);
size_t utf8_col_of(const char *s, size_t len, size_t byte_i, int tabstop);
size_t utf8_byte_at_col(const char *s, size_t len, size_t col, int tabstop);
int utf8_encode(uint32_t cp, char out[4]);

#endif
