#include "tack/cli.h"
#include "test.h"

void test_cli_no_args(void)
{
    char *argv[] = {"tack"};
    TackCli cli = tack_cli_parse(1, argv);
    ASSERT_EQ_INT("run action", TACK_CLI_RUN, cli.action);
    ASSERT_NULL("no filename", cli.filename);
}

void test_cli_filename(void)
{
    char *argv[] = {"tack", "notes.txt"};
    TackCli cli = tack_cli_parse(2, argv);
    ASSERT_EQ_INT("run action", TACK_CLI_RUN, cli.action);
    ASSERT_STREQ("filename", "notes.txt", cli.filename);
}

void test_cli_help(void)
{
    char *argv[] = {"tack", "--help"};
    TackCli cli = tack_cli_parse(2, argv);
    ASSERT_EQ_INT("help long", TACK_CLI_HELP, cli.action);

    argv[1] = "-h";
    cli = tack_cli_parse(2, argv);
    ASSERT_EQ_INT("help short", TACK_CLI_HELP, cli.action);
}

void test_cli_version(void)
{
    char *argv[] = {"tack", "--version"};
    TackCli cli = tack_cli_parse(2, argv);
    ASSERT_EQ_INT("version long", TACK_CLI_VERSION, cli.action);

    argv[1] = "-v";
    cli = tack_cli_parse(2, argv);
    ASSERT_EQ_INT("version short", TACK_CLI_VERSION, cli.action);
}

void test_cli_unknown_option(void)
{
    char *argv[] = {"tack", "--bogus"};
    TackCli cli = tack_cli_parse(2, argv);
    ASSERT_EQ_INT("error", TACK_CLI_ERROR, cli.action);
    ASSERT_NOT_NULL("error message", cli.error);
}

void test_cli_too_many_files(void)
{
    char *argv[] = {"tack", "a.txt", "b.txt"};
    TackCli cli = tack_cli_parse(3, argv);
    ASSERT_EQ_INT("error", TACK_CLI_ERROR, cli.action);
}
