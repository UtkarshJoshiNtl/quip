#include "quip.h"

int main() {
    printf("Starting quip 0.2...\n");
    
    history_init();
    terminal_init();
    prompt_init();

    while (1) {
        char *cmd_line = get_command_line();
        if (read_line(cmd_line, MAX_LINE) >= 0) {
            execute_line(cmd_line);
        }
    }

    prompt_cleanup();
    terminal_cleanup();
    history_cleanup();
    return 0;
}
