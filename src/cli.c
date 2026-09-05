#include "missnotepad/cli.h"
#include "missnotepad/version.h"

#include <stdio.h>
#include <string.h>

MissNotepadCli missnotepad_cli_parse(int argc, char **argv)
{
    MissNotepadCli cli;
    int i;

    cli.action = MISSNOTEPAD_CLI_RUN;
    cli.filename = NULL;
    cli.error = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            cli.action = MISSNOTEPAD_CLI_HELP;
            return cli;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            cli.action = MISSNOTEPAD_CLI_VERSION;
            return cli;
        }
        if (argv[i][0] == '-') {
            cli.action = MISSNOTEPAD_CLI_ERROR;
            cli.error = "unknown option";
            return cli;
        }
        if (cli.filename != NULL) {
            cli.action = MISSNOTEPAD_CLI_ERROR;
            cli.error = "too many file arguments";
            return cli;
        }
        cli.filename = argv[i];
    }

    return cli;
}

void missnotepad_cli_print_help(void)
{
    printf("Usage: miss [FILE]\n");
    printf("       miss --help\n");
    printf("       miss --version\n");
    printf("\n");
    printf("%s is a full-screen command-line text editor\n", MISSNOTEPAD_NAME);
    printf("(Microsoft Notepad for the Linux terminal).\n");
    printf("\n");
    printf("In the editor: type to insert, use the mouse or menus.\n");
    printf("F10 or Alt+letter opens the Notepad-style menus.\n");
    printf("Default keys are Notepad; switch themes under View.\n");
    printf("Ctrl+S save, Ctrl+Q quit (Notepad theme).\n");
}

void missnotepad_cli_print_version(void)
{
    printf("%s %s\n", MISSNOTEPAD_NAME, MISSNOTEPAD_VERSION_STRING);
}
