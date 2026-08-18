#ifndef TACK_KEYS_H
#define TACK_KEYS_H

#include <stddef.h>
#include <stdint.h>

#define MOD_SHIFT 1
#define MOD_ALT 2
#define MOD_CTRL 4

typedef enum {
    EV_NONE = 0,
    EV_KEY,
    EV_RESIZE,
    EV_MOUSE
} EventKind;

typedef enum {
    KEY_CHAR = 0,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_ESC,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_HOME,
    KEY_END,
    KEY_PGUP,
    KEY_PGDN,
    KEY_INSERT,
    KEY_DELETE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12
} KeyId;

typedef struct {
    EventKind kind;
    KeyId key;
    uint32_t ch;
    int mods;
    int mx;
    int my;
    int mbtn;
    int mdown;
} Event;

int event_parse(const unsigned char *p, size_t n, int allow_incomplete, Event *out);

#endif
