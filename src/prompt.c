#include "quip.h"

static char comline[MAX_LINE];

char *get_command_line(void) {
    return comline;
}

void print_prompt(void) {
    char cwd[1024];
    char *home = getenv("HOME");
    char hostname[256];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strncpy(cwd, "???", sizeof(cwd) - 1);
        cwd[sizeof(cwd) - 1] = '\0';
    }

    if (gethostname(hostname, sizeof(hostname)) == -1) {
        strncpy(hostname, "localhost", sizeof(hostname) - 1);
        hostname[sizeof(hostname) - 1] = '\0';
    }

    char *username = getenv("USER");
    if (!username) username = "user";

    printf(ANSI_CYAN "┌──[" ANSI_YELLOW "%s" ANSI_WHITE "@" ANSI_RED "%s" ANSI_CYAN "]\n", username, hostname);

    if (home != NULL && strncmp(cwd, home, strlen(home)) == 0) {
        printf(ANSI_CYAN "├──[" ANSI_BLUE "~%s" ANSI_CYAN "]\n", cwd + strlen(home));
    } else {
        printf(ANSI_CYAN "├──[" ANSI_BLUE "%s" ANSI_CYAN "]\n", cwd);
    }

    printf(ANSI_CYAN "└─╼" ANSI_GREEN "➤ " ANSI_RESET);
}

int read_line(char *buffer, size_t size) {
    if (!isatty(STDIN_FILENO)) {
        if (fgets(buffer, size, stdin)) {
            buffer[strcspn(buffer, "\n")] = '\0';
            if (buffer[0] != '\0')
                history_push(buffer);
            return strlen(buffer);
        }
        return -1;
    }

    print_prompt();
    fflush(stdout);

    int pos = 0;
    int len = 0;
    int hist_idx = history_size();
    char buf[MAX_LINE];
    char saved[MAX_LINE];

    buf[0] = '\0';
    saved[0] = '\0';

    while (1) {
        char c;
        ssize_t result = read(STDIN_FILENO, &c, 1);
        if (result <= 0) break;

        if (c == '\n' || c == '\r') {
            write(STDOUT_FILENO, "\n", 1);
            buf[len] = '\0';
            break;
        }

        if (c == 127 || c == '\b') {
            if (pos > 0) {
                memmove(buf + pos - 1, buf + pos, len - pos);
                pos--;
                len--;
                buf[len] = '\0';

                write(STDOUT_FILENO, "\r", 1);
                print_prompt();
                printf("%.*s ", len, buf);
                printf("\r");
                print_prompt();
                printf("%.*s", pos, buf);
                fflush(stdout);
            }
            continue;
        }

        if (c == '\x1b') {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

            if (seq[0] == '[') {
                if (seq[1] == 'A' || seq[1] == 'B') {
                    if (hist_idx == history_size())
                        strncpy(saved, buf, MAX_LINE - 1);
                    saved[MAX_LINE - 1] = '\0';

                    if (seq[1] == 'A' && hist_idx > 0)
                        hist_idx--;
                    else if (seq[1] == 'B' && hist_idx < history_size())
                        hist_idx++;

                    const char *entry = (hist_idx == history_size()) ? saved : history_get(hist_idx);
                    if (entry) {
                        strncpy(buf, entry, MAX_LINE - 1);
                        buf[MAX_LINE - 1] = '\0';
                        len = pos = strlen(buf);
                    }

                    printf("\r\033[2K");
                    print_prompt();
                    printf("%s", buf);
                    fflush(stdout);

                } else if (seq[1] == 'C' && pos < len) {
                    pos++;
                    write(STDOUT_FILENO, "\033[C", 3);
                } else if (seq[1] == 'D' && pos > 0) {
                    pos--;
                    write(STDOUT_FILENO, "\033[D", 3);
                }
            }
            continue;
        }

        if (c >= 32 && c < 127) {
            if (len < MAX_LINE - 1) {
                memmove(buf + pos + 1, buf + pos, len - pos);
                buf[pos] = c;
                pos++;
                len++;
                buf[len] = '\0';

                write(STDOUT_FILENO, buf + pos - 1, len - pos + 1);

                if (len - pos > 0) {
                    printf("\033[%dD", len - pos);
                    fflush(stdout);
                }
            }
        } else if (c == '\t') {
            handle_completion(buf, &pos, &len);
        }
    }

    buf[strcspn(buf, "\n")] = '\0';
    if (buf[0] != '\0')
        history_push(buf);

    strncpy(buffer, buf, size - 1);
    buffer[size - 1] = '\0';

    return len;
}

void prompt_init(void) {
    comline[0] = '\0';
}

void prompt_cleanup(void) {
    comline[0] = '\0';
}
