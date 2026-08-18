#include "tack/buffer.h"
#include "test.h"

#include <stdlib.h>
#include <string.h>

static void assert_buffer(const char *msg, Buffer *b, const char *expected)
{
    size_t n = 0;
    char *got = buf_to_string(b, &n);
    ASSERT_NOT_NULL(msg, got);
    ASSERT_STREQ(msg, expected, got);
    free(got);
}

void test_buffer_empty(void)
{
    Buffer b;
    size_t len = 99;
    ASSERT_EQ_INT("init", 0, buf_init(&b));
    ASSERT_EQ_INT("one line", 1, (int)buf_line_count(&b));
    ASSERT_STREQ("empty line", "", buf_line(&b, 0, &len));
    ASSERT_EQ_INT("len 0", 0, (int)len);
    assert_buffer("to_string empty", &b, "");
    buf_free(&b);
}

void test_buffer_insert_chars(void)
{
    Buffer b;
    buf_init(&b);
    ASSERT_EQ_INT("insert hi", 0, buf_insert_cstr(&b, 0, 0, "hi"));
    assert_buffer("hi", &b, "hi");
    ASSERT_EQ_INT("insert middle", 0, buf_insert_cstr(&b, 0, 1, "ey "));
    assert_buffer("hey i", &b, "hey i");
    buf_free(&b);
}

void test_buffer_newlines(void)
{
    Buffer b;
    buf_init(&b);
    ASSERT_EQ_INT("insert", 0, buf_insert_cstr(&b, 0, 0, "one\ntwo\nthree"));
    ASSERT_EQ_INT("3 lines", 3, (int)buf_line_count(&b));
    ASSERT_STREQ("line0", "one", buf_line(&b, 0, NULL));
    ASSERT_STREQ("line1", "two", buf_line(&b, 1, NULL));
    ASSERT_STREQ("line2", "three", buf_line(&b, 2, NULL));
    assert_buffer("roundtrip", &b, "one\ntwo\nthree");
    buf_free(&b);
}

void test_buffer_crlf(void)
{
    Buffer b;
    buf_init(&b);
    ASSERT_EQ_INT("load crlf", 0, buf_load_mem(&b, "a\r\nb\r\n", 6));
    ASSERT_EQ_INT("3 lines (trailing nl)", 3, (int)buf_line_count(&b));
    ASSERT_STREQ("a", "a", buf_line(&b, 0, NULL));
    ASSERT_STREQ("b", "b", buf_line(&b, 1, NULL));
    ASSERT_STREQ("empty last", "", buf_line(&b, 2, NULL));
    buf_free(&b);
}

void test_buffer_split_join(void)
{
    Buffer b;
    buf_init(&b);
    buf_insert_cstr(&b, 0, 0, "hello world");
    ASSERT_EQ_INT("split", 0, buf_split(&b, 0, 6));
    ASSERT_EQ_INT("2 lines", 2, (int)buf_line_count(&b));
    ASSERT_STREQ("hello ", "hello ", buf_line(&b, 0, NULL));
    ASSERT_STREQ("world", "world", buf_line(&b, 1, NULL));
    ASSERT_EQ_INT("join", 0, buf_join_next(&b, 0));
    ASSERT_EQ_INT("1 line", 1, (int)buf_line_count(&b));
    assert_buffer("joined", &b, "hello world");
    buf_free(&b);
}

void test_buffer_delete(void)
{
    Buffer b;
    buf_init(&b);
    buf_insert_cstr(&b, 0, 0, "abcdef");
    ASSERT_EQ_INT("delete", 0, buf_delete(&b, 0, 2, 3));
    assert_buffer("abf", &b, "abf");
    buf_free(&b);
}

void test_buffer_load_replace(void)
{
    Buffer b;
    buf_init(&b);
    buf_insert_cstr(&b, 0, 0, "old");
    ASSERT_EQ_INT("load", 0, buf_load_mem(&b, "new\ntext", 8));
    assert_buffer("replaced", &b, "new\ntext");
    buf_free(&b);
}

void test_buffer_span(void)
{
    Buffer b;
    size_t n = 0;
    char *s;
    buf_init(&b);
    buf_insert_cstr(&b, 0, 0, "hello\nworld");
    s = buf_copy_span(&b, 0, 3, 1, 2, &n);
    ASSERT_STREQ("copy span", "lo\nwo", s);
    ASSERT_EQ_INT("span len", 5, (int)n);
    free(s);
    ASSERT_EQ_INT("delete span", 0, buf_delete_span(&b, 0, 3, 1, 2));
    assert_buffer("after span del", &b, "helrld");
    buf_free(&b);
}

void test_buffer_split_at_ends(void)
{
    Buffer b;
    buf_init(&b);
    buf_insert_cstr(&b, 0, 0, "ab");
    ASSERT_EQ_INT("split start", 0, buf_split(&b, 0, 0));
    ASSERT_EQ_INT("2 lines", 2, (int)buf_line_count(&b));
    ASSERT_STREQ("empty first", "", buf_line(&b, 0, NULL));
    ASSERT_STREQ("ab", "ab", buf_line(&b, 1, NULL));
    buf_free(&b);
}
