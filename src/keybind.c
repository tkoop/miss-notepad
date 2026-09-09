#include "missnotepad/keybind.h"

#include <string.h>

void keybind_state_init(KeybindState *st)
{
    st->emacs_cx = 0;
    st->vi_mode = VI_INSERT;
}

static KeyCmd act(Action a)
{
    KeyCmd c;
    c.kind = CMD_ACTION;
    c.action = a;
    return c;
}

static KeyCmd pass(void)
{
    KeyCmd c;
    c.kind = CMD_PASS;
    c.action = ACT_NONE;
    return c;
}

static KeyCmd consume(void)
{
    KeyCmd c;
    c.kind = CMD_CONSUME;
    c.action = ACT_NONE;
    return c;
}

static int ctrl(const Event *ev, char ch)
{
    return ev->kind == EV_KEY && ev->key == KEY_CHAR && ev->mods == MOD_CTRL &&
           ev->ch == (uint32_t)ch;
}

static int chr(const Event *ev, char ch)
{
    return ev->kind == EV_KEY && ev->key == KEY_CHAR && ev->mods == 0 &&
           ev->ch == (uint32_t)ch;
}

static KeyCmd map_notepad(const Event *ev)
{
    if (ctrl(ev, 'q')) {
        return act(ACT_EXIT);
    }
    if (ctrl(ev, 's')) {
        return act(ACT_SAVE);
    }
    if (ctrl(ev, 'n')) {
        return act(ACT_NEW);
    }
    if (ctrl(ev, 'o')) {
        return act(ACT_OPEN);
    }
    if (ctrl(ev, 'z')) {
        return act(ACT_UNDO);
    }
    if (ctrl(ev, 'y')) {
        return act(ACT_REDO);
    }
    if (ctrl(ev, 'x')) {
        return act(ACT_CUT);
    }
    if (ctrl(ev, 'c')) {
        return act(ACT_COPY);
    }
    if (ctrl(ev, 'v')) {
        return act(ACT_PASTE);
    }
    if (ctrl(ev, 'a')) {
        return act(ACT_SELECT_ALL);
    }
    if (ctrl(ev, 'f')) {
        return act(ACT_FIND);
    }
    if (ctrl(ev, 'h')) {
        return act(ACT_REPLACE);
    }
    if (ev->key == KEY_F3) {
        return act(ACT_FIND_NEXT);
    }
    return pass();
}

static KeyCmd map_nano(const Event *ev)
{
    if (ctrl(ev, 'x')) {
        return act(ACT_EXIT);
    }
    if (ctrl(ev, 'o')) {
        return act(ACT_SAVE);
    }
    if (ctrl(ev, 's')) {
        return act(ACT_SAVE);
    }
    if (ctrl(ev, 'w')) {
        return act(ACT_FIND);
    }
    if (ctrl(ev, '\\')) {
        return act(ACT_REPLACE);
    }
    if (ctrl(ev, 'k')) {
        return act(ACT_KILL_LINE);
    }
    if (ctrl(ev, 'u')) {
        return act(ACT_PASTE);
    }
    if (ctrl(ev, 'g')) {
        return act(ACT_HELP_KEYS);
    }
    if (ctrl(ev, 'a')) {
        return pass(); /* let editor treat as... we don't have start-of-file here */
    }
    return pass();
}

static KeyCmd map_emacs(const Event *ev, KeybindState *st)
{
    if (st->emacs_cx) {
        st->emacs_cx = 0;
        if (ctrl(ev, 's')) {
            return act(ACT_SAVE);
        }
        if (ctrl(ev, 'c')) {
            return act(ACT_EXIT);
        }
        if (ctrl(ev, 'f')) {
            return act(ACT_OPEN);
        }
        if (ctrl(ev, 'w')) {
            return act(ACT_SAVE_AS);
        }
        return consume();
    }
    if (ctrl(ev, 'x')) {
        st->emacs_cx = 1;
        return consume();
    }
    if (ctrl(ev, 's')) {
        return act(ACT_FIND);
    }
    if (ctrl(ev, 'r')) {
        return act(ACT_FIND_NEXT);
    }
    if (ctrl(ev, 'y')) {
        return act(ACT_PASTE);
    }
    if (ctrl(ev, 'w')) {
        return act(ACT_CUT);
    }
    if (ctrl(ev, 'k')) {
        return act(ACT_KILL_LINE);
    }
    if (ctrl(ev, 'a')) {
        return act(ACT_MOVE_HOME);
    }
    if (ctrl(ev, 'e')) {
        return act(ACT_MOVE_END);
    }
    if (ctrl(ev, 'f')) {
        return act(ACT_MOVE_RIGHT);
    }
    if (ctrl(ev, 'b')) {
        return act(ACT_MOVE_LEFT);
    }
    if (ctrl(ev, 'n')) {
        return act(ACT_MOVE_DOWN);
    }
    if (ctrl(ev, 'p')) {
        return act(ACT_MOVE_UP);
    }
    return pass();
}

static KeyCmd map_vi(const Event *ev, KeybindState *st)
{
    if (st->vi_mode == VI_INSERT) {
        if (ev->key == KEY_ESC) {
            st->vi_mode = VI_NORMAL;
            return consume();
        }
        if (ctrl(ev, 's')) {
            return act(ACT_SAVE);
        }
        return pass();
    }
    /* normal mode */
    if (ev->key == KEY_ESC) {
        return consume();
    }
    if (chr(ev, 'i')) {
        st->vi_mode = VI_INSERT;
        return consume();
    }
    if (chr(ev, 'a')) {
        st->vi_mode = VI_INSERT;
        return act(ACT_MOVE_RIGHT);
    }
    if (chr(ev, 'x')) {
        return act(ACT_DELETE);
    }
    if (chr(ev, 'p')) {
        return act(ACT_PASTE);
    }
    if (chr(ev, 'u')) {
        return act(ACT_UNDO);
    }
    if (chr(ev, '/')) {
        return act(ACT_FIND);
    }
    if (chr(ev, 'n')) {
        return act(ACT_FIND_NEXT);
    }
    if (chr(ev, ':')) {
        return act(ACT_VI_COLON);
    }
    if (chr(ev, 'h')) {
        return act(ACT_MOVE_LEFT);
    }
    if (chr(ev, 'j')) {
        return act(ACT_MOVE_DOWN);
    }
    if (chr(ev, 'k')) {
        return act(ACT_MOVE_UP);
    }
    if (chr(ev, 'l')) {
        return act(ACT_MOVE_RIGHT);
    }
    if (ev->key == KEY_LEFT || ev->key == KEY_RIGHT || ev->key == KEY_UP ||
        ev->key == KEY_DOWN || ev->key == KEY_HOME || ev->key == KEY_END ||
        ev->key == KEY_PGUP || ev->key == KEY_PGDN) {
        return pass();
    }
    return consume();
}

KeyCmd keybind_map(KeyTheme theme, const Event *ev, KeybindState *st)
{
    if (ev == NULL || ev->kind != EV_KEY) {
        return pass();
    }
    switch (theme) {
    case THEME_NANO:
        return map_nano(ev);
    case THEME_EMACS:
        return map_emacs(ev, st);
    case THEME_VI:
        return map_vi(ev, st);
    case THEME_NOTEPAD:
    default:
        return map_notepad(ev);
    }
}

const char *keybind_help(KeyTheme theme)
{
    switch (theme) {
    case THEME_NANO:
        return "nano: ^O save  ^X exit  ^W find  ^K cut  ^U paste  ^\\ replace";
    case THEME_EMACS:
        return "Emacs: C-x C-s save  C-x C-c quit  C-s find  C-y paste  C-w cut";
    case THEME_VI:
        return "vi: Esc normal  i insert  hjkl move  x del  p paste  / find  :w :q";
    case THEME_NOTEPAD:
    default:
        return "Notepad: Ctrl+S save  Ctrl+O open  Ctrl+F find  Ctrl+Z undo  Ctrl+Q quit";
    }
}

/* Short key hints for the status line, per theme. */
const char *keybind_status_hint(KeyTheme theme)
{
    switch (theme) {
    case THEME_NANO:
        return "^O save  ^W find  ^K cut  ^X exit";
    case THEME_EMACS:
        return "C-x C-s save  C-s find  C-y paste  C-x C-c quit";
    case THEME_VI:
        return "i insert  Esc normal  :w save  :q quit";
    case THEME_NOTEPAD:
    default:
        return "Ctrl+S save  Ctrl+Z undo  Ctrl+Q quit";
    }
}
