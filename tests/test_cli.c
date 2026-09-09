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

void test_cli_keys(void)
{
    char *argv[] = {"miss", "--keys=nano", "notes.txt"};
    MissNotepadCli cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("keys run", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_EQ_INT("keys set", 1, cli.keys_set);
    ASSERT_EQ_INT("keys nano", THEME_NANO, cli.keys_theme);

    argv[1] = "--keys";
    cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("keys missing arg", MISSNOTEPAD_CLI_ERROR, cli.action);

    argv[1] = "--keys";
    argv[2] = "vi";
    cli = missnotepad_cli_parse(3, argv);
    ASSERT_EQ_INT("keys vi run", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_EQ_INT("keys vi set", 1, cli.keys_set);
    ASSERT_EQ_INT("keys vi", THEME_VI, cli.keys_theme);

    argv[1] = "--keys=emacs";
    argv[2] = "notes.txt";
    cli = missnotepad_cli_parse(3, argv);
    ASSERT_EQ_INT("keys emacs run", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_EQ_INT("keys emacs", THEME_EMACS, cli.keys_theme);
    ASSERT_STREQ("keys with file", "notes.txt", cli.filename);

    argv[1] = "--keys=bogus";
    cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("keys bogus", MISSNOTEPAD_CLI_ERROR, cli.action);
    ASSERT_NOT_NULL("keys bogus msg", cli.error);
}

void test_cli_wrap(void)
{
    char *argv[] = {"miss", "--no-wrap", "--keys=vim"};
    MissNotepadCli cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("no-wrap run", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_EQ_INT("no-wrap set", 1, cli.wrap_set);
    ASSERT_EQ_INT("no-wrap off", 0, cli.wrap);

    argv[1] = "--wrap";
    cli = missnotepad_cli_parse(2, argv);
    ASSERT_EQ_INT("wrap run", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_EQ_INT("wrap set", 1, cli.wrap_set);
    ASSERT_EQ_INT("wrap on", 1, cli.wrap);

    argv[1] = "--wrap";
    argv[2] = "--keys=vim";
    cli = missnotepad_cli_parse(3, argv);
    ASSERT_EQ_INT("combined run", MISSNOTEPAD_CLI_RUN, cli.action);
    ASSERT_EQ_INT("combined wrap", 1, cli.wrap_set);
    ASSERT_EQ_INT("combined theme", THEME_VI, cli.keys_theme);
}
