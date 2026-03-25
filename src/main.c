#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_LINE    256
#define MAX_ARGS     64
#define MAX_HISTORY  64

char comline[MAX_LINE];
char *args[MAX_ARGS];


char *history[MAX_HISTORY];
int   history_len = 0;

void history_push(const char *line) {
    if (history_len > 0 && strcmp(history[history_len - 1], line) == 0)
        return;
    if (history_len == MAX_HISTORY) {
        free(history[0]);
        memmove(history, history + 1, (MAX_HISTORY - 1) * sizeof(char *));
        history_len--;
    }
    history[history_len++] = strdup(line);
}



static struct termios orig_termios;

static void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}



void startup() {
    printf("Starting quip 0.2...\n");
    enable_raw_mode();
}

void prompter() {
    printf("User%%-> ");
    fflush(stdout);

    int    pos      = 0;
    int    len      = 0;
    int    hist_idx = history_len;
    char   buf[MAX_LINE];
    char   saved[MAX_LINE];
    buf[0] = '\0';
    saved[0] = '\0';

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;

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
                printf("User%%-> %.*s", len, buf);
                printf(" ");
                printf("\r");
                printf("User%%-> %.*s", pos, buf);
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
                    if (hist_idx == history_len)
                        strncpy(saved, buf, MAX_LINE);

                    if (seq[1] == 'A' && hist_idx > 0)              hist_idx--;
                    else if (seq[1] == 'B' && hist_idx < history_len) hist_idx++;

                    const char *entry = (hist_idx == history_len) ? saved : history[hist_idx];
                    strncpy(buf, entry, MAX_LINE - 1);
                    len = pos = strlen(buf);

                    printf("\r\033[2K");
                    printf("User%%-> %s", buf);
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
                /* redraw from cursor */
                write(STDOUT_FILENO, buf + pos - 1, len - pos + 1);
                /* move cursor back to pos */
                if (len - pos > 0) {
                    printf("\033[%dD", len - pos);
                    fflush(stdout);
                }
            }
        }
    }

    
    buf[strcspn(buf, "\n")] = '\0';
    if (buf[0] != '\0')
        history_push(buf);

    strncpy(comline, buf, MAX_LINE - 1);
    comline[MAX_LINE - 1] = '\0';
}


typedef int (*builtin_func)(char **argv);

struct builtin {
    const char *name;
    builtin_func func;
};

int builtin_cd(char **argv) {
    if (argv[1] != NULL) {
        if (chdir(argv[1]) != 0)
            perror("chdir failed");
    } else {
        fprintf(stderr, "cd: missing operand\n");
    }
    return 1;
}

int builtin_pwd(char **argv) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\n", cwd);
    else
        perror("getcwd failed");
    return 1;
}

int builtin_help(char **argv) {
    printf("Available builtins:\n");
    extern struct builtin builtins[];
    for (int i = 0; builtins[i].name != NULL; i++)
        printf("  %s\n", builtins[i].name);
    return 1;
}

int builtin_history(char **argv) {
    for (int i = 0; i < history_len; i++)
        printf("  %3d  %s\n", i + 1, history[i]);
    return 1;
}

int builtin_exit(char **argv) {
    return -1;
}

struct builtin builtins[] = {
    { "cd",      builtin_cd      },
    { "pwd",     builtin_pwd     },
    { "help",    builtin_help    },
    { "history", builtin_history },
    { "exit",    builtin_exit    },
    { NULL,      NULL            }
};

builtin_func find_builtin(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0)
            return builtins[i].func;
    }
    return NULL;
}


int parse_line(char *line, char **argv) {
    int argc = 0;
    char *p  = line;

    while (*p) {
        if (argc >= MAX_ARGS - 1) break;

        while (*p == ' ' || *p == '\n') p++;
        if (*p == '\0') break;

        if (*p == '"') {
            p++;
            argv[argc++] = p;
            while (*p && *p != '"') {
                if (*p == '\\' && *(p+1)) p++;
                p++;
            }
            if (*p) *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\n') {
                if (*p == '\\' && *(p+1))
                    memmove(p, p+1, strlen(p));
                p++;
            }
            if (*p) *p++ = '\0';
        }
    }

    argv[argc] = NULL;
    return argc;
}

void handle_redirection(char **argv) {
    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            if (argv[i+1] == NULL) {
                fprintf(stderr, "quip: missing filename after '%s'\n", argv[i]);
                continue;
            }
            int flags = O_WRONLY | O_CREAT | (argv[i][1] == '>' ? O_APPEND : O_TRUNC);
            int fd = open(argv[i+1], flags, 0644);
            if (fd < 0) { perror("open failed"); continue; }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            argv[i] = NULL;
            argv[i+1] = NULL;
        } else if (strcmp(argv[i], "<") == 0) {
            if (argv[i+1] == NULL) {
                fprintf(stderr, "quip: missing filename after '<'\n");
                continue;
            }
            int fd = open(argv[i+1], O_RDONLY);
            if (fd < 0) { perror("open failed"); continue; }
            dup2(fd, STDIN_FILENO);
            close(fd);
            argv[i] = NULL;
            argv[i+1] = NULL;
        }
    }
}

void execute_single(char *cmd) {
    parse_line(cmd, args);
    if (args[0] == NULL) return;

    builtin_func bf = find_builtin(args[0]);
    if (bf) {
        int res = bf(args);
        if (res == -1) exit(0);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }
    if (pid == 0) {
        handle_redirection(args);
        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

void execute_line(char *line) {
    char *cmd = strtok(line, ";");
    while (cmd != NULL) {
        execute_single(cmd);
        cmd = strtok(NULL, ";");
    }
}

int main() {
    startup();
    while (1) {
        prompter();
        execute_line(comline);
    }
    return 0;
}
