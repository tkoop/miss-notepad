#include "tack/screen.h"
#include "test.h"

void test_screen_put_get(void)
{
    Screen s;
    screen_init(&s);
    ASSERT_EQ_INT("resize", 0, screen_resize(&s, 3, 10));
    ASSERT_EQ_INT("space", (int)' ', (int)screen_get(&s, 0, 0));
    screen_put(&s, 1, 2, (uint32_t)'X', STYLE_TITLE);
    ASSERT_EQ_INT("X", (int)'X', (int)screen_get(&s, 1, 2));
    ASSERT_EQ_INT("style", STYLE_TITLE, screen_get_style(&s, 1, 2));
    screen_puts(&s, 0, 0, "Hi", STYLE_NORMAL);
    ASSERT_EQ_INT("H", (int)'H', (int)screen_get(&s, 0, 0));
    ASSERT_EQ_INT("i", (int)'i', (int)screen_get(&s, 0, 1));
    screen_free(&s);
}

void test_screen_clips(void)
{
    Screen s;
    screen_init(&s);
    screen_resize(&s, 2, 4);
    screen_puts(&s, 0, 0, "HELLO", STYLE_NORMAL);
    ASSERT_EQ_INT("H", (int)'H', (int)screen_get(&s, 0, 0));
    ASSERT_EQ_INT("L last visible", (int)'L', (int)screen_get(&s, 0, 3));
    screen_put(&s, 9, 9, (uint32_t)'Z', STYLE_NORMAL);
    ASSERT_EQ_INT("oob", 0, (int)screen_get(&s, 9, 9));
    screen_free(&s);
}
