#ifndef TACK_FILEIO_H
#define TACK_FILEIO_H

#include <stddef.h>

/* 0 = ok, 1 = not found, -1 = error (errno set). */
int file_read_all(const char *path, char **data, size_t *len);

#endif
