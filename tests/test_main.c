#include "test.h"

void test_version_string(void);
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
        {"cli_no_args", test_cli_no_args},
        {"cli_filename", test_cli_filename},
        {"cli_help", test_cli_help},
        {"cli_version", test_cli_version},
        {"cli_unknown_option", test_cli_unknown_option},
        {"cli_too_many_files", test_cli_too_many_files},
    };

    return tack_run_tests(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
