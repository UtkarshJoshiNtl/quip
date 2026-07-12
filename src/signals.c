#include "quip.h"
#include <signal.h>

static volatile sig_atomic_t should_exit = 0;
static volatile sig_atomic_t sigint_received = 0;

static void signal_handler(int sig) {
    switch (sig) {
        case SIGINT:
            if (isatty(STDOUT_FILENO))
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
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, ANSI_DIM "quip: " ANSI_RED "sigaction SIGTERM failed: %s\n" ANSI_RESET, strerror(errno));
    }

    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, ANSI_DIM "quip: " ANSI_RED "sigaction SIGINT failed: %s\n" ANSI_RESET, strerror(errno));
    }

    signal(SIGQUIT, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
}

void signals_cleanup(void) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
}

void signal_exit_shell(void) {
    should_exit = 1;
}

int should_exit_shell(void) {
    return should_exit;
}

int was_sigint_received(void) {
    sigset_t block, old;
    sigemptyset(&block);
    sigaddset(&block, SIGINT);
    sigprocmask(SIG_BLOCK, &block, &old);
    int ret = sigint_received;
    if (ret) sigint_received = 0;
    sigprocmask(SIG_SETMASK, &old, NULL);
    return ret;
}
