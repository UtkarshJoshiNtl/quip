#include "quip.h"

extern void print_prompt(void);

int main(void) {
    config_init();
    signals_init();
    jobs_init();
    history_init();
    terminal_init();
    prompt_init();
    plugin_init();

    if (isatty(STDIN_FILENO)) {
        printf(ANSI_GREEN "╭─────────────────────────────────────────────────────╮\n");
        printf("│" ANSI_YELLOW "  ████████╗███████╗██████╗ ███╗   ███╗██╗███╗   ██╗ " ANSI_GREEN "│\n");
        printf("│" ANSI_YELLOW "  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║██║████╗  ██║ " ANSI_GREEN "│\n");
        printf("│" ANSI_YELLOW "     ██║   █████╗  ██████╔╝██╔████╔██║██║██╔██╗ ██║ " ANSI_GREEN "│\n");
        printf("│" ANSI_YELLOW "     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║██║██║╚██╗██║ " ANSI_GREEN "│\n");
        printf("│" ANSI_YELLOW "     ██║   ███████╗██║  ██║██║ ╚═╝ ██║██║██║ ╚████║ " ANSI_GREEN "│\n");
        printf("│" ANSI_YELLOW "     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝ " ANSI_GREEN "│\n");
        printf("│" ANSI_BLUE "                 Quip Shell v0.4" ANSI_GREEN "                  │\n");
        printf("╰─────────────────────────────────────────────────────╯\n");
        printf(ANSI_RED "★ " ANSI_WHITE "Welcome to " ANSI_GREEN "Quip" ANSI_WHITE " - A modern, enhanced shell experience!\n");
        printf(ANSI_RED "★ " ANSI_WHITE "Features: Pipes, History, Job Control, Auto-completion, Python Plugins\n");
        printf(ANSI_RED "★ " ANSI_WHITE "Type '" ANSI_BLUE "help" ANSI_WHITE "' for available commands\n\n" ANSI_RESET);
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
        printf(ANSI_RED "★ " ANSI_WHITE "Thank you for using " ANSI_GREEN "Quip" ANSI_WHITE "! Goodbye!\n" ANSI_RESET);
    }
    return 0;
}
