#include "missnotepad/term.h"

#include "missnotepad/utf8.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static struct termios g_orig;
static int g_raw;
static volatile sig_atomic_t g_resized;
static unsigned char g_inbuf[64];
static size_t g_inlen;

static void on_winch(int sig)
{
    (void)sig;
    g_resized = 1;
}

int term_available(void)
{
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
}

int term_size(int *rows, int *cols)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0) {
        return -1;
    }
    if (rows) {
        *rows = ws.ws_row > 0 ? ws.ws_row : 24;
    }
    if (cols) {
        *cols = ws.ws_col > 0 ? ws.ws_col : 80;
    }
    return 0;
}

int term_init(void)
{
    struct termios raw;
    struct sigaction sa;

    if (!term_available()) {
        return -1;
    }
    if (tcgetattr(STDIN_FILENO, &g_orig) != 0) {
        return -1;
    }
    raw = g_orig;
    raw.c_iflag &= (unsigned)~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= (unsigned)~(OPOST);
    raw.c_cflag |= (unsigned)CS8;
    raw.c_lflag &= (unsigned)~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        return -1;
    }
    g_raw = 1;
    g_inlen = 0;
    g_resized = 0;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_winch;
    sigaction(SIGWINCH, &sa, NULL);

    /* Alternate screen, hide cursor, clear. */
    static const char enter[] =
        "\033[?1049h\033[?25l\033[2J\033[H\033[?1000h\033[?1002h\033[?1006h";
    if (write(STDOUT_FILENO, enter, sizeof(enter) - 1) < 0) {
        term_shutdown();
        return -1;
    }
    atexit(term_shutdown);
    return 0;
}

void term_shutdown(void)
{
    if (!g_raw) {
        return;
    }
    /* "\033[0 q" resets DECSCUSR so we don't leave a bar cursor behind. */
    static const char leave[] =
        "\033[?1006l\033[?1002l\033[?1000l\033[0 q\033[?25h\033[?1049l";
    if (write(STDOUT_FILENO, leave, sizeof(leave) - 1) < 0) {
        /* best-effort restore */
    }
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig);
    g_raw = 0;
}

static int fill_input(int timeout_ms)
{
    fd_set fds;
    struct timeval tv;
    ssize_t n;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, timeout_ms < 0 ? NULL : &tv) <=
        0) {
        return 0;
    }
    n = read(STDIN_FILENO, g_inbuf + g_inlen, sizeof(g_inbuf) - g_inlen);
    if (n > 0) {
        g_inlen += (size_t)n;
        return 1;
    }
    return 0;
}

int term_read_event(Event *ev)
{
    if (g_resized) {
        g_resized = 0;
        ev->kind = EV_RESIZE;
        ev->key = KEY_CHAR;
        ev->ch = 0;
        ev->mods = 0;
        ev->mx = 0;
        ev->my = 0;
        ev->mbtn = 0;
        ev->mdown = 0;
        return 1;
    }

    for (;;) {
        if (g_inlen > 0) {
            int used = event_parse(g_inbuf, g_inlen, 1, ev);
            if (used > 0) {
                memmove(g_inbuf, g_inbuf + used, g_inlen - (size_t)used);
                g_inlen -= (size_t)used;
                return 1;
            }
            if (g_inbuf[0] == 0x1b && g_inlen < sizeof(g_inbuf)) {
                fill_input(50);
                used = event_parse(g_inbuf, g_inlen, 0, ev);
                if (used > 0) {
                    memmove(g_inbuf, g_inbuf + used, g_inlen - (size_t)used);
                    g_inlen -= (size_t)used;
                    return 1;
                }
                memmove(g_inbuf, g_inbuf + 1, g_inlen - 1);
                g_inlen -= 1;
                continue;
            }
        }
        fill_input(200);
        if (g_resized) {
            g_resized = 0;
            ev->kind = EV_RESIZE;
            ev->key = KEY_CHAR;
            ev->ch = 0;
            ev->mods = 0;
            ev->mx = ev->my = ev->mbtn = ev->mdown = 0;
            return 1;
        }
        if (g_inlen == 0) {
            ev->kind = EV_NONE;
            return 0;
        }
    }
}

static void style_ansi(unsigned char style, char *out, size_t cap)
{
    const char *s = "\033[0m";
    switch (style) {
    case STYLE_TITLE:
    case STYLE_MENU:
        s = "\033[0;44;37m";
        break;
    case STYLE_STATUS:
        s = "\033[0;47;30m";
        break;
    case STYLE_INVERSE:
    case STYLE_MENU_SEL:
    case STYLE_BUTTON_FOCUS:
        s = "\033[0;7m";
        break;
    case STYLE_GUTTER:
        s = "\033[0;90m";
        break;
    case STYLE_MENU_HOT:
        s = "\033[0;44;37;1m";
        break;
    case STYLE_DIALOG:
        s = "\033[0;47;30m";
        break;
    case STYLE_DIALOG_TITLE:
        s = "\033[0;44;37;1m";
        break;
    case STYLE_BUTTON:
        s = "\033[0;47;30m";
        break;
    case STYLE_FIELD:
    case STYLE_FIELD_FOCUS:
        s = "\033[0;40;37m";
        break;
    case STYLE_SELECT:
        s = "\033[0;47;30m";
        break;
    default:
        s = "\033[0m";
        break;
    }
    strncpy(out, s, cap - 1);
    out[cap - 1] = '\0';
}

void term_flush(const Screen *s)
{
    char buf[8192];
    size_t used = 0;
    int y, x;
    unsigned char last = 255;
    char cur_style[24];

    /* Home, then paint. */
#define FLUSH()                                                                \
    do {                                                                       \
        if (used > 0) {                                                        \
            if (write(STDOUT_FILENO, buf, used) < 0) {                         \
                used = 0;                                                      \
                return;                                                        \
            }                                                                  \
            used = 0;                                                          \
        }                                                                      \
    } while (0)
#define APPEND(p, n)                                                           \
    do {                                                                       \
        size_t _n = (size_t)(n);                                               \
        if (used + _n > sizeof(buf)) {                                         \
            FLUSH();                                                           \
        }                                                                      \
        memcpy(buf + used, (p), _n);                                           \
        used += _n;                                                            \
    } while (0)

    APPEND("\033[H", 3);
    for (y = 0; y < s->rows; y++) {
        if (y > 0) {
            APPEND("\r\n", 2);
        }
        for (x = 0; x < s->cols; x++) {
            const Cell *c = &s->cells[y * s->cols + x];
            char u[4];
            int un;
            if (c->ch == CELL_CONT) {
                continue;
            }
            if (c->style != last) {
                style_ansi(c->style, cur_style, sizeof(cur_style));
                APPEND(cur_style, strlen(cur_style));
                last = c->style;
            }
            if (c->ch == 0 || c->ch == (uint32_t)' ') {
                APPEND(" ", 1);
            } else {
                un = utf8_encode(c->ch, u);
                if (un <= 0) {
                    APPEND("?", 1);
                } else {
                    APPEND(u, un);
                }
            }
        }
        APPEND("\033[K", 3);
    }
    APPEND("\033[0m", 4);
    if (s->show_cursor) {
        /* DECSCUSR "\033[5 q" = blinking vertical bar (insert cursor). */
        char pos[32];
        int n = snprintf(pos, sizeof(pos), "\033[%d;%dH\033[?25h\033[5 q",
                         s->cy + 1, s->cx + 1);
        APPEND(pos, n);
    } else {
        APPEND("\033[?25l", 6);
    }
    FLUSH();
#undef APPEND
#undef FLUSH
}
