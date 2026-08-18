#ifndef TACK_TERM_H
#define TACK_TERM_H

#include "tack/keys.h"
#include "tack/screen.h"

int term_available(void);
int term_init(void);
void term_shutdown(void);
int term_size(int *rows, int *cols);
int term_read_event(Event *ev);
void term_flush(const Screen *s);

#endif
