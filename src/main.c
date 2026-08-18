#include "tack/cli.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    TackCli cli = tack_cli_parse(argc, argv);

    if (cli.action == TACK_CLI_HELP) {
        tack_cli_print_help();
        return 0;
    }
    if (cli.action == TACK_CLI_VERSION) {
        tack_cli_print_version();
        return 0;
    }
    if (cli.action == TACK_CLI_ERROR) {
        fprintf(stderr, "tack: %s\n", cli.error ? cli.error : "invalid arguments");
        fprintf(stderr, "Try 'tack --help' for more information.\n");
        return 2;
    }

    printf("Tack %s — full-screen editor coming in a later version.\n",
           cli.filename ? cli.filename : "(untitled)");
    (void)cli;
    return 0;
}
