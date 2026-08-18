#include "tack/cli.h"
#include "tack/editor.h"
#include "tack/settings.h"
#include "tack/term.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_editor(const char *filename)
{
    Editor ed;
    Screen scr;
    int rows = 24;
    int cols = 80;

    if (!term_available()) {
        fprintf(stderr, "tack: a terminal is required\n");
        return 1;
    }
    if (editor_init(&ed) != 0) {
        fprintf(stderr, "tack: out of memory\n");
        return 1;
    }
    {
        Settings set;
        const char *home = getenv("HOME");
        char path[512];
        settings_defaults(&set);
        if (home != NULL) {
            snprintf(path, sizeof(path), "%s/.config/tack/config", home);
            settings_load(&set, path);
        }
        editor_apply_settings(&ed, &set);
    }
    if (filename != NULL) {
        if (editor_load_path(&ed, filename) != 0) {
            fprintf(stderr, "tack: cannot open '%s'\n", filename);
            editor_free(&ed);
            return 1;
        }
    }
    if (term_init() != 0) {
        fprintf(stderr, "tack: failed to initialize the terminal\n");
        editor_free(&ed);
        return 1;
    }
    screen_init(&scr);
    term_size(&rows, &cols);
    if (screen_resize(&scr, rows, cols) != 0) {
        term_shutdown();
        editor_free(&ed);
        return 1;
    }
    editor_set_view(&ed, rows > 2 ? rows - 2 : 1, cols);

    while (!ed.quit) {
        Event ev;
        editor_render(&ed, &scr);
        term_flush(&scr);
        if (term_read_event(&ev)) {
            if (ev.kind == EV_RESIZE) {
                if (term_size(&rows, &cols) == 0) {
                    screen_resize(&scr, rows, cols);
                    editor_set_view(&ed, rows > 2 ? rows - 2 : 1, cols);
                }
            } else {
                editor_handle_event(&ed, &ev);
            }
        }
    }

    term_shutdown();
    screen_free(&scr);
    editor_free(&ed);
    return 0;
}

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

    return run_editor(cli.filename);
}
