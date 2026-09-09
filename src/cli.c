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
    cli.keys_set = 0;
    cli.keys_theme = THEME_NOTEPAD;
    cli.wrap_set = 0;
    cli.wrap = 1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            cli.action = MISSNOTEPAD_CLI_HELP;
            return cli;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            cli.action = MISSNOTEPAD_CLI_VERSION;
            return cli;
        }
        if (strcmp(argv[i], "--wrap") == 0) {
            cli.wrap_set = 1;
            cli.wrap = 1;
            continue;
        }
        if (strcmp(argv[i], "--no-wrap") == 0) {
            cli.wrap_set = 1;
            cli.wrap = 0;
            continue;
        }
        if (strcmp(argv[i], "--keys") == 0) {
            if (i + 1 >= argc) {
                cli.action = MISSNOTEPAD_CLI_ERROR;
                cli.error = "option '--keys' requires a theme argument";
                return cli;
            }
            i++;
            if (settings_theme_from_name(argv[i], &cli.keys_theme) != 0) {
                cli.action = MISSNOTEPAD_CLI_ERROR;
                cli.error = "unknown key theme";
                return cli;
            }
            cli.keys_set = 1;
            continue;
        }
        if (strncmp(argv[i], "--keys=", 7) == 0) {
            if (settings_theme_from_name(argv[i] + 7, &cli.keys_theme) != 0) {
                cli.action = MISSNOTEPAD_CLI_ERROR;
                cli.error = "unknown key theme";
                return cli;
            }
            cli.keys_set = 1;
            continue;
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
    printf("Usage: miss [OPTIONS] [FILE]\n");
    printf("       miss --help\n");
    printf("       miss --version\n");
    printf("\n");
    printf("%s is a full-screen command-line text editor\n", MISSNOTEPAD_NAME);
    printf("(Microsoft Notepad for the Linux terminal).\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help            Show this help and exit\n");
    printf("  -v, --version         Show version and exit\n");
    printf("  --keys=THEME          Key binding theme: notepad, nano, vi or emacs\n");
    printf("                        (saved to the config file)\n");
    printf("  --wrap, --no-wrap     Turn word wrap on or off (saved to the\n");
    printf("                        config file)\n");
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
