#ifndef TACK_CLI_H
#define TACK_CLI_H

typedef enum {
    TACK_CLI_RUN = 0,
    TACK_CLI_HELP = 1,
    TACK_CLI_VERSION = 2,
    TACK_CLI_ERROR = 3
} TackCliAction;

typedef struct {
    TackCliAction action;
    const char *filename;
    const char *error;
} TackCli;

TackCli tack_cli_parse(int argc, char **argv);
void tack_cli_print_help(void);
void tack_cli_print_version(void);

#endif
