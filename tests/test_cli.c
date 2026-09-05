#include "missnotepad/cli.h"
#include "test.h"

void test_cli_no_args(void)
{
    char *argv[] = {"miss"};
    MissNotepadCli cli = missnotepad_cli_parse(1, argv);
    ASSERT_EQ_INT("run action", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_NULL("no filename", cli.filename);
}

void test_cli_filename(void)
{
    char *argv[] = {"miss", "notes.txt"};
    MissNotepadCli cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("run action", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_STREQ("filename", "notes.txt", cli.filename);
}

void test_cli_help(void)
{
    char *argv[] = {"miss", "--help"};
    MissNotepadCli cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("help long", MISSNOTEPAD_CLI_HELP, cli.action);

    argv[1] = "-h";
    cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("help short", MISSNOTEPAD_CLI_HELP, cli.action);
}

void test_cli_version(void)
{
    char *argv[] = {"miss", "--version"};
    MissNotepadCli cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("version long", MISSNOTEPAD_CLI_VERSION, cli.action);

    argv[1] = "-v";
    cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("version short", MISSNOTEPAD_CLI_VERSION, cli.action);
}

void test_cli_unknown_option(void)
{
    char *argv[] = {"miss", "--bogus"};
    MissNotepadCli cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("error", MISSNOTEPAD_CLI_ERROR, cli.action);
    ASSERT_NOT_NULL("error message", cli.error);
}

void test_cli_too_many_files(void)
{
    char *argv[] = {"miss", "a.txt", "b.txt"};
    MissNotepadCli cli = missnotepad_cli_parse(3, argv);
    ASSERT_EQ_INT("error", MISSNOTEPAD_CLI_ERROR, cli.action);
}
