#include "tack/utf8.h"

static int utf8_is_cont(unsigned char c)
{
    return (c & 0xC0) == 0x80;
}

size_t utf8_next(const char *s, size_t len, size_t i)
{
    if (i >= len) {
        return len;
    }
    i++;
    while (i < len && utf8_is_cont((unsigned char)s[i])) {
        i++;
    }
    return i;
}

size_t utf8_prev(const char *s, size_t len, size_t i)
{
    (void)len;
    if (i == 0) {
        return 0;
    }
    i--;
    while (i > 0 && utf8_is_cont((unsigned char)s[i])) {
        i--;
    }
    return i;
}

int utf8_decode(const char *s, size_t len, size_t i, uint32_t *cp, size_t *nbytes)
{
    unsigned char c;
    uint32_t u;
    size_t n;
    size_t k;

    if (i >= len) {
        return -1;
    }
    c = (unsigned char)s[i];
    if (c < 0x80) {
        u = c;
        n = 1;
    } else if ((c & 0xE0) == 0xC0 && i + 1 < len) {
        u = (uint32_t)(c & 0x1F) << 6;
        n = 2;
    } else if ((c & 0xF0) == 0xE0 && i + 2 < len) {
        u = (uint32_t)(c & 0x0F) << 12;
        n = 3;
    } else if ((c & 0xF8) == 0xF0 && i + 3 < len) {
        u = (uint32_t)(c & 0x07) << 18;
        n = 4;
    } else {
        if (cp) {
            *cp = 0xFFFD;
        }
        if (nbytes) {
            *nbytes = 1;
        }
        return -1;
    }
    for (k = 1; k < n; k++) {
        unsigned char cc = (unsigned char)s[i + k];
        if (!utf8_is_cont(cc)) {
            if (cp) {
                *cp = 0xFFFD;
            }
            if (nbytes) {
                *nbytes = 1;
            }
            return -1;
        }
        u |= (uint32_t)(cc & 0x3F) << (6 * (n - 1 - k));
    }
    if (cp) {
        *cp = u;
    }
    if (nbytes) {
        *nbytes = n;
    }
    return 0;
}

int utf8_codepoint_width(uint32_t cp)
{
    if (cp == 0 || cp == 0x7F) {
        return 0;
    }
    if (cp < 32) {
        return 0;
    }
    /* East Asian wide / fullwidth ranges commonly seen in terminals. */
    if ((cp >= 0x1100 && cp <= 0x115F) || (cp >= 0x2329 && cp <= 0x232A) ||
        (cp >= 0x2E80 && cp <= 0xA4CF && cp != 0x303F) ||
        (cp >= 0xAC00 && cp <= 0xD7A3) || (cp >= 0xF900 && cp <= 0xFAFF) ||
        (cp >= 0xFE10 && cp <= 0xFE19) || (cp >= 0xFE30 && cp <= 0xFE6F) ||
        (cp >= 0xFF00 && cp <= 0xFF60) || (cp >= 0xFFE0 && cp <= 0xFFE6) ||
        (cp >= 0x1F300 && cp <= 0x1F64F) || (cp >= 0x1F900 && cp <= 0x1F9FF)) {
        return 2;
    }
    return 1;
}

size_t utf8_display_width(const char *s, size_t len, size_t bytes, int tabstop)
{
    size_t i = 0;
    size_t width = 0;
    int ts = tabstop > 0 ? tabstop : TACK_TABSTOP_DEFAULT;

    if (bytes > len) {
        bytes = len;
    }
    while (i < bytes) {
        uint32_t cp;
        size_t n;
        if (utf8_decode(s, len, i, &cp, &n) != 0) {
            width += 1;
            i += 1;
            continue;
        }
        if (cp == '\t') {
            size_t next = ((width / (size_t)ts) + 1) * (size_t)ts;
            width = next;
        } else {
            int w = utf8_codepoint_width(cp);
            width += (size_t)(w > 0 ? w : 1);
        }
        i += n;
    }
    return width;
}

size_t utf8_col_of(const char *s, size_t len, size_t byte_i, int tabstop)
{
    if (byte_i > len) {
        byte_i = len;
    }
    return utf8_display_width(s, len, byte_i, tabstop);
}

size_t utf8_byte_at_col(const char *s, size_t len, size_t col, int tabstop)
{
    size_t i = 0;
    size_t width = 0;
    int ts = tabstop > 0 ? tabstop : TACK_TABSTOP_DEFAULT;

    while (i < len) {
        uint32_t cp;
        size_t n;
        size_t next_width;

        if (utf8_decode(s, len, i, &cp, &n) != 0) {
            n = 1;
            cp = (unsigned char)s[i];
        }
        if (cp == '\t') {
            next_width = ((width / (size_t)ts) + 1) * (size_t)ts;
        } else {
            int w = utf8_codepoint_width(cp);
            next_width = width + (size_t)(w > 0 ? w : 1);
        }
        if (next_width > col) {
            break;
        }
        width = next_width;
        i += n;
        if (width == col) {
            break;
        }
    }
    return i;
}

int utf8_encode(uint32_t cp, char out[4])
{
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    }
    if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | (cp >> 6));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    }
    if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | (cp >> 12));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    }
    if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | (cp >> 18));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return 0;
}
