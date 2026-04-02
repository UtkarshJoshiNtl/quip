#include "quip.h"

int main() {
    printf("Starting quip 0.2...\n");
    
    signals_init();
    jobs_init();
    history_init();
    terminal_init();
    prompt_init();

    while (!should_exit_shell()) {
        cleanup_jobs();
        char *cmd_line = get_command_line();
        if (read_line(cmd_line, MAX_LINE) >= 0) {
            execute_line(cmd_line);
        }
    }

    prompt_cleanup();
    terminal_cleanup();
    history_cleanup();
    signals_cleanup();
    printf("\nExiting quip...\n");
    return 0;
}
