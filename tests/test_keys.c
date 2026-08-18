#include "tack/keys.h"
#include "test.h"

static Event parse_str(const char *s)
{
    Event ev;
    int used = event_parse((const unsigned char *)s, strlen(s), 0, &ev);
    (void)used;
    return ev;
}

void test_keys_ctrl_q(void)
{
    unsigned char q = 0x11; /* Ctrl+Q */
    Event ev;
    ASSERT_EQ_INT("consumed", 1, event_parse(&q, 1, 0, &ev));
    ASSERT_EQ_INT("kind", EV_KEY, ev.kind);
    ASSERT_EQ_INT("char", KEY_CHAR, ev.key);
    ASSERT_EQ_INT("q", (int)'q', (int)ev.ch);
    ASSERT_EQ_INT("ctrl", MOD_CTRL, ev.mods);
}

void test_keys_arrows(void)
{
    Event ev = parse_str("\033[A");
    ASSERT_EQ_INT("up", KEY_UP, ev.key);
    ev = parse_str("\033[B");
    ASSERT_EQ_INT("down", KEY_DOWN, ev.key);
    ev = parse_str("\033[C");
    ASSERT_EQ_INT("right", KEY_RIGHT, ev.key);
    ev = parse_str("\033[D");
    ASSERT_EQ_INT("left", KEY_LEFT, ev.key);
}

void test_keys_ctrl_arrow(void)
{
    Event ev = parse_str("\033[1;5C");
    ASSERT_EQ_INT("right", KEY_RIGHT, ev.key);
    ASSERT_EQ_INT("ctrl", MOD_CTRL, ev.mods);
}

void test_keys_home_end_delete(void)
{
    Event ev = parse_str("\033[H");
    ASSERT_EQ_INT("home", KEY_HOME, ev.key);
    ev = parse_str("\033[F");
    ASSERT_EQ_INT("end", KEY_END, ev.key);
    ev = parse_str("\033[3~");
    ASSERT_EQ_INT("delete", KEY_DELETE, ev.key);
    ev = parse_str("\033[5~");
    ASSERT_EQ_INT("pgup", KEY_PGUP, ev.key);
}

void test_keys_incomplete_esc(void)
{
    unsigned char esc = 0x1b;
    Event ev;
    ASSERT_EQ_INT("need more", 0, event_parse(&esc, 1, 1, &ev));
    ASSERT_EQ_INT("lone esc", 1, event_parse(&esc, 1, 0, &ev));
    ASSERT_EQ_INT("esc key", KEY_ESC, ev.key);
}

void test_keys_utf8_char(void)
{
    const char *s = "\xC3\xA9";
    Event ev;
    ASSERT_EQ_INT("consumed 2", 2,
                  event_parse((const unsigned char *)s, 2, 0, &ev));
    ASSERT_EQ_INT("e-acute", 0xE9, (int)ev.ch);
}
