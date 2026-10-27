#include "quip.h"

static char comline[MAX_LINE];

char *get_command_line(void) {
    return comline;
}

void print_prompt(void) {
    printf("\r");
    char cwd[1024];
    char *home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strncpy(cwd, "???", sizeof(cwd) - 1);
        cwd[sizeof(cwd) - 1] = '\0';
    }

    char *username = getenv("USER");
    if (!username) username = "user";

    char pretty[1024];
    size_t home_len = home ? strlen(home) : 0;
    if (home && home_len > 0 && strncmp(cwd, home, home_len) == 0 &&
        (cwd[home_len] == '/' || cwd[home_len] == '\0'))
        snprintf(pretty, sizeof(pretty), "~%s", cwd + home_len);
    else
        snprintf(pretty, sizeof(pretty), "%s", cwd);

    printf(ANSI_ACCENT "%s" ANSI_DIM " " ANSI_ACCENT "%s" ANSI_DIM " > " ANSI_RESET,
           username, pretty);
}

int read_line(char *buffer, size_t size) {
    if (!isatty(STDIN_FILENO)) {
        if (fgets(buffer, size, stdin)) {
            buffer[strcspn(buffer, "\n")] = '\0';
            return (int)strlen(buffer);
        }
        return -1;
    }

    print_prompt();
    fflush(stdout);

    if (!fgets(buffer, size, stdin)) {
        if (errno == EINTR) return -1;
        return -2;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    buffer[strcspn(buffer, "\r")] = '\0';
    return (int)strlen(buffer);
}

void prompt_init(void) {
    comline[0] = '\0';
}

void prompt_cleanup(void) {
    comline[0] = '\0';
}
