#include "missnotepad/clipboard.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    const char *program;
    char *const *argv;
} ClipboardCommand;

static int run_command(const ClipboardCommand *command, const char *text,
                       size_t len)
{
    int pipefd[2];
    pid_t pid;
    size_t offset = 0;
    int write_failed = 0;
    int status;
    struct sigaction ignore = {0};
    struct sigaction old_action;

    if (pipe(pipefd) != 0) {
        return -1;
    }
    pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    if (pid == 0) {
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            _exit(127);
        }
        close(pipefd[0]);
        execvp(command->program, command->argv);
        _exit(127);
    }

    close(pipefd[0]);
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    sigaction(SIGPIPE, &ignore, &old_action);
    while (offset < len) {
        ssize_t written = write(pipefd[1], text + offset, len - offset);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            write_failed = 1;
            break;
        }
        offset += (size_t)written;
    }
    close(pipefd[1]);
    sigaction(SIGPIPE, &old_action, NULL);

    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            return -1;
        }
    }
    if (write_failed || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return -1;
    }
    return 0;
}

int missnotepad_clipboard_set(const char *text, size_t len)
{
    static char *const wl_copy_argv[] = {"wl-copy", NULL};
    static char *const xclip_argv[] = {"xclip", "-selection", "clipboard", NULL};
    static char *const xsel_argv[] = {"xsel", "--clipboard", "--input", NULL};
    ClipboardCommand commands[3];
    size_t command_count = 0;
    size_t i;

    if (text == NULL && len != 0) {
        return -1;
    }
    if (getenv("WAYLAND_DISPLAY") != NULL) {
        commands[command_count++] = (ClipboardCommand){"wl-copy", wl_copy_argv};
    }
    if (getenv("DISPLAY") != NULL) {
        commands[command_count++] = (ClipboardCommand){"xclip", xclip_argv};
        commands[command_count++] = (ClipboardCommand){"xsel", xsel_argv};
    }
    if (command_count == 0) {
        commands[command_count++] = (ClipboardCommand){"wl-copy", wl_copy_argv};
        commands[command_count++] = (ClipboardCommand){"xclip", xclip_argv};
        commands[command_count++] = (ClipboardCommand){"xsel", xsel_argv};
    }
    for (i = 0; i < command_count; i++) {
        if (run_command(&commands[i], text ? text : "", len) == 0) {
            return 0;
        }
    }
    return -1;
}