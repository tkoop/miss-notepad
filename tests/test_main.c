#include "test.h"

void test_version_string(void);
void test_utf8_ascii_next_prev(void);
void test_utf8_multibyte(void);
void test_utf8_tab_width(void);
void test_utf8_byte_at_col(void);
void test_buffer_empty(void);
void test_buffer_insert_chars(void);
void test_buffer_newlines(void);
void test_buffer_crlf(void);
void test_buffer_split_join(void);
void test_buffer_delete(void);
void test_buffer_load_replace(void);
void test_buffer_split_at_ends(void);
void test_cli_no_args(void);
void test_cli_filename(void);
void test_cli_help(void);
void test_cli_version(void);
void test_cli_unknown_option(void);
void test_cli_too_many_files(void);

int main(void)
{
    const TackTest tests[] = {
        {"version_string", test_version_string},
        {"utf8_ascii_next_prev", test_utf8_ascii_next_prev},
        {"utf8_multibyte", test_utf8_multibyte},
        {"utf8_tab_width", test_utf8_tab_width},
        {"utf8_byte_at_col", test_utf8_byte_at_col},
        {"buffer_empty", test_buffer_empty},
        {"buffer_insert_chars", test_buffer_insert_chars},
        {"buffer_newlines", test_buffer_newlines},
        {"buffer_crlf", test_buffer_crlf},
        {"buffer_split_join", test_buffer_split_join},
        {"buffer_delete", test_buffer_delete},
        {"buffer_load_replace", test_buffer_load_replace},
        {"buffer_split_at_ends", test_buffer_split_at_ends},
        {"cli_no_args", test_cli_no_args},
        {"cli_filename", test_cli_filename},
        {"cli_help", test_cli_help},
        {"cli_version", test_cli_version},
        {"cli_unknown_option", test_cli_unknown_option},
        {"cli_too_many_files", test_cli_too_many_files},
    };

    return tack_run_tests(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
