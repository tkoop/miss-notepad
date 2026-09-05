#include "missnotepad/wrap.h"

#include "missnotepad/utf8.h"

size_t wrap_line_starts(const char *line, size_t len, int tabstop, int width,
                        size_t *starts, size_t max_starts)
{
    size_t i = 0;
    size_t n = 0;
    size_t col = 0;
    size_t last_break = 0;
    size_t last_break_col = 0;

    if (width < 2) {
        width = 2;
    }
    if (starts && max_starts > 0) {
        starts[0] = 0;
    }
    n = 1;

    while (i < len) {
        uint32_t cp = 0;
        size_t nbytes = 1;
        int cw;
        if (utf8_decode(line, len, i, &cp, &nbytes) != 0) {
            cp = (unsigned char)line[i];
            nbytes = 1;
        }
        if (cp == '\t') {
            int ts = tabstop > 0 ? tabstop : 4;
            cw = ts - (int)(col % (size_t)ts);
        } else {
            cw = utf8_codepoint_width(cp);
            if (cw <= 0) {
                cw = 1;
            }
        }
        if (col + (size_t)cw > (size_t)width && col > 0) {
            size_t br = (last_break > (starts && n > 0 ? starts[n - 1] : 0) &&
                         last_break_col > 0)
                            ? last_break
                            : i;
            if (starts && n < max_starts) {
                starts[n] = br;
            }
            n++;
            col = 0;
            last_break = 0;
            last_break_col = 0;
            i = br;
            continue;
        }
        if (cp == ' ' || cp == '\t') {
            last_break = i + nbytes;
            last_break_col = col + (size_t)cw;
        }
        col += (size_t)cw;
        i += nbytes;
    }
    return n == 0 ? 1 : n;
}
