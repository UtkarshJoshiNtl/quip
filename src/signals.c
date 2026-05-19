#include "quip.h"
#include <signal.h>

static volatile sig_atomic_t should_exit = 0;
static volatile sig_atomic_t sigint_received = 0;

static void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
            write(STDOUT_FILENO, "^C\n", 3);
            sigint_received = 1;
            break;
        case SIGTERM:
            should_exit = 1;
            break;
    }
}

void signals_init(void) {
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, ANSI_RED "sigaction SIGINT failed: %s\n" ANSI_RESET, strerror(errno));
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, ANSI_RED "sigaction SIGTERM failed: %s\n" ANSI_RESET, strerror(errno));
    }

    signal(SIGQUIT, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);
}

void signals_cleanup(void) {
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
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
