#include "quip.h"

int main(void) {
    signals_init();
    terminal_init();
    prompt_init();

    if (isatty(STDIN_FILENO)) {
        printf(ANSI_DIM "quip " ANSI_ACCENT "v0.1" ANSI_DIM "  \u2014  type " ANSI_BOLD "help" ANSI_DIM " for commands\n" ANSI_RESET);
    }

    while (!should_exit_shell()) {
        char *cmd_line = get_command_line();
        if (was_sigint_received()) {
            if (isatty(STDIN_FILENO)) {
                print_prompt();
                fflush(stdout);
            }
            continue;
        }
        int ret = read_line(cmd_line, MAX_LINE);
        if (ret == -2) break;
        if (ret < 0) {
            if (!isatty(STDIN_FILENO)) {
                break;
            }
            continue;
        }
        execute_line(cmd_line);
    }

    prompt_cleanup();
    terminal_cleanup();
    signals_cleanup();

    if (isatty(STDIN_FILENO)) {
        printf(ANSI_DIM "quip" ANSI_GRAY " \u2014 goodbye\n" ANSI_RESET);
    }
    return 0;
}
