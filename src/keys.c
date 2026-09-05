#include "missnotepad/keys.h"

#include "missnotepad/utf8.h"

static int mods_from_param(int p)
{
    int m = 0;
    if (p <= 1) {
        return 0;
    }
    p -= 1;
    if (p & 1) {
        m |= MOD_SHIFT;
    }
    if (p & 2) {
        m |= MOD_ALT;
    }
    if (p & 4) {
        m |= MOD_CTRL;
    }
    return m;
}

static KeyId key_from_tilde(int n)
{
    switch (n) {
    case 1:
    case 7:
        return KEY_HOME;
    case 2:
        return KEY_INSERT;
    case 3:
        return KEY_DELETE;
    case 4:
    case 8:
        return KEY_END;
    case 5:
        return KEY_PGUP;
    case 6:
        return KEY_PGDN;
    case 11:
        return KEY_F1;
    case 12:
        return KEY_F2;
    case 13:
        return KEY_F3;
    case 14:
        return KEY_F4;
    case 15:
        return KEY_F5;
    case 17:
        return KEY_F6;
    case 18:
        return KEY_F7;
    case 19:
        return KEY_F8;
    case 20:
        return KEY_F9;
    case 21:
        return KEY_F10;
    case 23:
        return KEY_F11;
    case 24:
        return KEY_F12;
    default:
        return KEY_CHAR;
    }
}

static KeyId key_from_letter(int c)
{
    switch (c) {
    case 'A':
        return KEY_UP;
    case 'B':
        return KEY_DOWN;
    case 'C':
        return KEY_RIGHT;
    case 'D':
        return KEY_LEFT;
    case 'H':
        return KEY_HOME;
    case 'F':
        return KEY_END;
    case 'P':
        return KEY_F1;
    case 'Q':
        return KEY_F2;
    case 'R':
        return KEY_F3;
    case 'S':
        return KEY_F4;
    default:
        return KEY_CHAR;
    }
}

static void set_key(Event *out, KeyId key, uint32_t ch, int mods)
{
    out->kind = EV_KEY;
    out->key = key;
    out->ch = ch;
    out->mods = mods;
    out->mx = 0;
    out->my = 0;
    out->mbtn = 0;
    out->mdown = 0;
}

int event_parse(const unsigned char *p, size_t n, int allow_incomplete, Event *out)
{
    unsigned char c;

    if (n == 0) {
        return 0;
    }
    c = p[0];

    if (c == 0x1b) {
        if (n == 1) {
            if (allow_incomplete) {
                return 0;
            }
            set_key(out, KEY_ESC, 0, 0);
            return 1;
        }
        if (p[1] == '[') {
            size_t i = 2;
            int params[4] = {0, 0, 0, 0};
            int np = 0;
            int val = 0;
            int have = 0;

            if (n == 2 && allow_incomplete) {
                return 0;
            }
            if (i < n && p[i] == '<') {
                int btn = 0, mx = 0, my = 0;
                int field = 0;
                int v = 0;
                i++;
                while (i < n && p[i] != 'M' && p[i] != 'm') {
                    if (p[i] >= '0' && p[i] <= '9') {
                        v = v * 10 + (p[i] - '0');
                    } else if (p[i] == ';') {
                        if (field == 0) {
                            btn = v;
                        } else if (field == 1) {
                            mx = v;
                        }
                        field++;
                        v = 0;
                    }
                    i++;
                }
                if (i >= n) {
                    return allow_incomplete ? 0 : 1;
                }
                if (field >= 2) {
                    my = v;
                }
                set_key(out, KEY_CHAR, 0, 0);
                out->kind = EV_MOUSE;
                out->mbtn = btn;
                out->mx = mx > 0 ? mx - 1 : 0;
                out->my = my > 0 ? my - 1 : 0;
                out->mdown = p[i] == 'M';
                return (int)(i + 1);
            }
            while (i < n) {
                unsigned char ch = p[i];
                if (ch >= '0' && ch <= '9') {
                    val = val * 10 + (ch - '0');
                    have = 1;
                    i++;
                    continue;
                }
                if (ch == ';') {
                    if (np < 4) {
                        params[np++] = have ? val : 0;
                    }
                    val = 0;
                    have = 0;
                    i++;
                    continue;
                }
                if (ch >= 0x40 && ch <= 0x7E) {
                    if (have && np < 4) {
                        params[np++] = val;
                    }
                    if (ch == '~') {
                        set_key(out, key_from_tilde(params[0]), 0,
                                np >= 2 ? mods_from_param(params[1]) : 0);
                    } else {
                        set_key(out, key_from_letter(ch), 0,
                                np >= 2 ? mods_from_param(params[1]) : 0);
                    }
                    return (int)(i + 1);
                }
                break;
            }
            return allow_incomplete ? 0 : 1;
        }
        if (p[1] == 'O') {
            if (n < 3) {
                return allow_incomplete ? 0 : 1;
            }
            set_key(out, key_from_letter(p[2]), 0, 0);
            return 3;
        }
        /* Alt + key */
        {
            Event inner;
            int used = event_parse(p + 1, n - 1, allow_incomplete, &inner);
            if (used == 0) {
                return 0;
            }
            if (used < 0) {
                return used;
            }
            *out = inner;
            out->mods |= MOD_ALT;
            return used + 1;
        }
    }

    if (c == 0x7f || c == 0x08) {
        set_key(out, KEY_BACKSPACE, 0, 0);
        return 1;
    }
    if (c == '\r' || c == '\n') {
        set_key(out, KEY_ENTER, 0, 0);
        return 1;
    }
    if (c == '\t') {
        set_key(out, KEY_TAB, 0, 0);
        return 1;
    }
    if (c < 32) {
        if (c == 0) {
            set_key(out, KEY_CHAR, ' ', MOD_CTRL);
            return 1;
        }
        set_key(out, KEY_CHAR, (uint32_t)('a' + c - 1), MOD_CTRL);
        return 1;
    }

    {
        uint32_t cp = 0;
        size_t nbytes = 0;
        if (utf8_decode((const char *)p, n, 0, &cp, &nbytes) != 0) {
            if (c >= 0x80 && n < 4 && allow_incomplete) {
                return 0;
            }
            set_key(out, KEY_CHAR, c, 0);
            return 1;
        }
        set_key(out, KEY_CHAR, cp, 0);
        return (int)nbytes;
    }
}
