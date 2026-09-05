#include "missnotepad/utf8.h"
#include "test.h"

void test_utf8_ascii_next_prev(void)
{
    const char *s = "abc";
    ASSERT_EQ_INT("next 0", 1, (int)utf8_next(s, 3, 0));
    ASSERT_EQ_INT("next 1", 2, (int)utf8_next(s, 3, 1));
    ASSERT_EQ_INT("next end", 3, (int)utf8_next(s, 3, 3));
    ASSERT_EQ_INT("prev 3", 2, (int)utf8_prev(s, 3, 3));
    ASSERT_EQ_INT("prev 0", 0, (int)utf8_prev(s, 3, 0));
}

void test_utf8_multibyte(void)
{
    /* "héllo" — é is C3 A9 */
    const char s[] = "h\xC3\xA9llo";
    size_t len = 6;
    size_t i = utf8_next(s, len, 0);
    ASSERT_EQ_INT("after h", 1, (int)i);
    i = utf8_next(s, len, i);
    ASSERT_EQ_INT("after e-acute", 3, (int)i);
    i = utf8_prev(s, len, i);
    ASSERT_EQ_INT("back to e-acute", 1, (int)i);
    ASSERT_EQ_INT("width of prefix he", 2, (int)utf8_col_of(s, len, 3, 4));
}

void test_utf8_tab_width(void)
{
    const char *s = "\tA";
    ASSERT_EQ_INT("tab to 4", 4, (int)utf8_display_width(s, 2, 1, 4));
    ASSERT_EQ_INT("tab+A", 5, (int)utf8_display_width(s, 2, 2, 4));
    ASSERT_EQ_INT("byte at col 4", 1, (int)utf8_byte_at_col(s, 2, 4, 4));
}

void test_utf8_byte_at_col(void)
{
    const char *s = "hello";
    ASSERT_EQ_INT("col 0", 0, (int)utf8_byte_at_col(s, 5, 0, 4));
    ASSERT_EQ_INT("col 3", 3, (int)utf8_byte_at_col(s, 5, 3, 4));
    ASSERT_EQ_INT("col past end", 5, (int)utf8_byte_at_col(s, 5, 99, 4));
}
