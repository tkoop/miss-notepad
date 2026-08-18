#include "tack/cli.h"
#include "tack/version.h"

#include <stdio.h>
#include <string.h>

TackCli tack_cli_parse(int argc, char **argv)
{
    TackCli cli;
    int i;

    cli.action = TACK_CLI_RUN;
    cli.filename = NULL;
    cli.error = NULL;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            cli.action = TACK_CLI_HELP;
            return cli;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            cli.action = TACK_CLI_VERSION;
            return cli;
        }
        if (argv[i][0] == '-') {
            cli.action = TACK_CLI_ERROR;
            cli.error = "unknown option";
            return cli;
        }
        if (cli.filename != NULL) {
            cli.action = TACK_CLI_ERROR;
            cli.error = "too many file arguments";
            return cli;
        }
        cli.filename = argv[i];
    }

    return cli;
}

void tack_cli_print_help(void)
{
    printf("Usage: tack [FILE]\n");
    printf("       tack --help\n");
    printf("       tack --version\n");
    printf("\n");
    printf("%s is a full-screen command-line text editor\n", TACK_NAME);
    printf("(Microsoft Notepad for the Linux terminal).\n");
}

void tack_cli_print_version(void)
{
    printf("%s %s\n", TACK_NAME, TACK_VERSION_STRING);
}
