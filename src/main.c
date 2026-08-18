#include "tack/app.h"
#include "tack/cli.h"
#include "tack/term.h"

#include <stdio.h>

static int run_app(const char *filename)
{
    App app;
    Screen scr;
    int rows = 24;
    int cols = 80;

    if (!term_available()) {
        fprintf(stderr, "tack: a terminal is required\n");
        return 1;
    }
    if (app_init(&app, filename) != 0) {
        fprintf(stderr, "tack: cannot start%s%s\n", filename ? " " : "",
                filename ? filename : "");
        app_free(&app);
        return 1;
    }
    app_load_config(&app, NULL);
    if (term_init() != 0) {
        fprintf(stderr, "tack: failed to initialize the terminal\n");
        app_free(&app);
        return 1;
    }
    screen_init(&scr);
    if (term_size(&rows, &cols) != 0) {
        rows = 24;
        cols = 80;
    }
    if (screen_resize(&scr, rows, cols) != 0) {
        term_shutdown();
        app_free(&app);
        return 1;
    }
    editor_set_view(&app.editor, rows > 2 ? rows - 2 : 1, cols);

    while (!app.editor.quit) {
        Event ev;
        app_render(&app, &scr);
        term_flush(&scr);
        if (term_read_event(&ev)) {
            if (ev.kind == EV_RESIZE) {
                if (term_size(&rows, &cols) == 0) {
                    screen_resize(&scr, rows, cols);
                    editor_set_view(&app.editor, rows > 2 ? rows - 2 : 1, cols);
                    menubar_layout(&app.menu);
                }
            } else {
                app_handle_event(&app, &ev);
            }
        }
    }

    term_shutdown();
    screen_free(&scr);
    app_free(&app);
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

    return run_app(cli.filename);
}
