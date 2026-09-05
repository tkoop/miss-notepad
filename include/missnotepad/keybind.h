#ifndef MISSNOTEPAD_KEYBIND_H
#define MISSNOTEPAD_KEYBIND_H

#include "missnotepad/action.h"
#include "missnotepad/keys.h"
#include "missnotepad/settings.h"

#define VI_INSERT 0
#define VI_NORMAL 1

typedef enum {
    CMD_PASS = 0,
    CMD_ACTION,
    CMD_CONSUME
} CmdKind;

typedef struct {
    int emacs_cx;
    int vi_mode;
} KeybindState;

typedef struct {
    CmdKind kind;
    Action action;
} KeyCmd;

void keybind_state_init(KeybindState *st);
KeyCmd keybind_map(KeyTheme theme, const Event *ev, KeybindState *st);
const char *keybind_help(KeyTheme theme);

#endif
