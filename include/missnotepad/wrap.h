#ifndef MISSNOTEPAD_WRAP_H
#define MISSNOTEPAD_WRAP_H

#include <stddef.h>

/* Fill starts[] with byte offsets of visual rows for one logical line.
   Returns the number of visual rows (at least 1). */
size_t wrap_line_starts(const char *line, size_t len, int tabstop, int width,
                        size_t *starts, size_t max_starts);

#endif
