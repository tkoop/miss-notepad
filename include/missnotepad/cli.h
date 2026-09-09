#ifndef MISSNOTEPAD_CLI_H
#define MISSNOTEPAD_CLI_H

#include "missnotepad/settings.h"

typedef enum {
    MISSNOTEPAD_CLI_RUN = 0,
    MISSNOTEPAD_CLI_HELP = 1,
    MISSNOTEPAD_CLI_VERSION = 2,
    MISSNOTEPAD_CLI_ERROR = 3
} MissNotepadCliAction;

typedef struct {
    MissNotepadCliAction action;
    const char *filename;
    const char *error;
    /* --keys=THEME */
    int keys_set;
    KeyTheme keys_theme;
    /* --wrap / --no-wrap */
    int wrap_set;
    int wrap;
} MissNotepadCli;

MissNotepadCli missnotepad_cli_parse(int argc, char **argv);
void missnotepad_cli_print_help(void);
void missnotepad_cli_print_version(void);

#endif
