#include "quip.h"
#include <signal.h>

static volatile sig_atomic_t should_exit = 0;
static volatile sig_atomic_t sigint_received = 0;

void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
            write(STDOUT_FILENO, "^C\n", 3);
            should_exit = 0;
            sigint_received = 1;
            break;
        case SIGTERM:
            should_exit = 1;
            break;
        case SIGCHLD:
            while (waitpid(-1, NULL, WNOHANG) > 0);
            break;
    }
}

void signals_init(void) {
    struct sigaction sa;
    
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT failed");
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM failed");
    }
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction SIGCHLD failed");
    }
    
    signal(SIGQUIT, SIG_IGN);
}

void signals_cleanup(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}

int should_exit_shell(void) {
    return should_exit;
}

int was_sigint_received(void) {
    if (sigint_received) {
        sigint_received = 0;
        return 1;
    }
    return 0;
}
