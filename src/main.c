#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE   256
#define MAX_ARGS    64

char comline[MAX_LINE];
char *args[MAX_ARGS];

void startup() {
    printf("Starting quip 0.2...\n");
}

void prompter() {
    printf("User%%-> ");
    fflush(stdout);s
    fgets(comline, sizeof(comline), stdin);
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

int builtin_exit(char **argv) {
    return -1;
}

struct builtin builtins[] = {
    { "cd",   builtin_cd   },
    { "pwd",  builtin_pwd  },
    { "help", builtin_help },
    { "exit", builtin_exit },
    { NULL,   NULL         }
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
    char *p = line;

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
                if (*p == '\\' && *(p+1)) {
                    memmove(p, p+1, strlen(p));
                }
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