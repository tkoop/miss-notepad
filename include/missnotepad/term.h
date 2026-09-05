#ifndef MISSNOTEPAD_TERM_H
#define MISSNOTEPAD_TERM_H

#include "missnotepad/keys.h"
#include "missnotepad/screen.h"

int term_available(void);
int term_init(void);
void term_shutdown(void);
int term_size(int *rows, int *cols);
int term_read_event(Event *ev);
void term_flush(const Screen *s);

#endif
