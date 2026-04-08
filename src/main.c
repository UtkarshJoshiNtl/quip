#include "quip.h"

extern void print_prompt(void);

int main() {
    printf("\033[38;5;46m╭─────────────────────────────────────────────────────╮\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m  ████████╗███████╗██████╗ ███╗   ███╗██╗███╗   ██╗ \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m  ╚══██╔══╝██╔════╝██╔══██╗████╗ ████║██║████╗  ██║ \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m     ██║   █████╗  ██████╔╝██╔████╔██║██║██╔██╗ ██║ \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m     ██║   ██╔══╝  ██╔══██╗██║╚██╔╝██║██║██║╚██╗██║ \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m     ██║   ███████╗██║  ██║██║ ╚═╝ ██║██║██║ ╚████║ \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m     ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝ \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;51m                 Enhanced Shell v0.3              \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m╰─────────────────────────────────────────────────────╯\033[0m\n");
    printf("\033[38;5;196m★ \033[38;5;255mWelcome to \033[38;5;46mQuip\033[38;5;255m - A modern, enhanced shell experience!\033[0m\n");
    printf("\033[38;5;196m★ \033[38;5;255mFeatures: Pipes, History, Job Control, Auto-completion\033[0m\n");
    printf("\033[38;5;196m★ \033[38;5;255mType '\033[38;5;51mhelp\033[38;5;255m' for available commands\033[0m\n\n");
    
    signals_init();
    jobs_init();
    history_init();
    terminal_init();
    prompt_init();

    while (!should_exit_shell()) {
        cleanup_jobs();
        char *cmd_line = get_command_line();
        if (was_sigint_received()) {
            print_prompt();
            fflush(stdout);
            continue;
        }
        if (read_line(cmd_line, MAX_LINE) >= 0) {
            execute_line(cmd_line);
        }
    }

    prompt_cleanup();
    terminal_cleanup();
    history_cleanup();
    signals_cleanup();
    printf("\033[38;5;196m★ \033[38;5;255mThank you for using \033[38;5;46mQuip\033[38;5;255m! Goodbye! 👋\033[0m\n");
    return 0;
}
