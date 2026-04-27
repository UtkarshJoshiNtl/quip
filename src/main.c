#include "quip.h"

int main(void) {
    config_init();
    signals_init();
    jobs_init();
    history_init();
    terminal_init();
    prompt_init();
    plugin_init();

    if (isatty(STDIN_FILENO)) {
        printf(ANSI_DIM "quip " ANSI_ACCENT "v0.4" ANSI_DIM "  \u2014  type " ANSI_BOLD "help" ANSI_DIM " for commands\n" ANSI_RESET);
    }

    while (!should_exit_shell()) {
        cleanup_jobs();
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
    plugin_cleanup();
    terminal_cleanup();
    history_cleanup();
    signals_cleanup();

    if (isatty(STDIN_FILENO)) {
        printf(ANSI_DIM "quip" ANSI_GRAY " \u2014 goodbye\n" ANSI_RESET);
    }
    return 0;
}
