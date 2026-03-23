#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

char comline[256];
char *argv[64];

void Startup(){
    printf("Starting quip 0.2...\n");
}

void prompter(){
    printf("User%%-> ");
    fgets(comline, sizeof(comline), stdin);
}

typedef int (*builtin_func)(char **argv);

struct builtin {
    const char *name;
    builtin_func func;
};

int builtin_cd(char **argv) {
    if (argv[1] != NULL) {
        if (chdir(argv[1]) != 0) {
            perror("chdir failed");
        }
    } else {
        fprintf(stderr, "cd: missing operand\n");
    }
    return 1;
}

int builtin_pwd(char **argv) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd failed");
    }
    return 1;
}

int builtin_help(char **argv) {
    printf("Not implemented yet.\n");
    return 1;
}

int builtin_exit(char **argv) {
    return -1;
}

struct builtin builtins[] = {
    { "cd", builtin_cd },
    { "pwd", builtin_pwd },
    { "help", builtin_help },
    { "exit", builtin_exit },
    { NULL, NULL }
};

builtin_func find_builtin(const char *name) {
    for (int i = 0; builtins[i].name != NULL; ++i) {
        if (strcmp(name, builtins[i].name) == 0)
            return builtins[i].func;
    }
    return NULL;
}

int parse_line(char *line, char **argv) {
    int argc = 0;
    char *p = line;

    while (*p) {
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
        if (strcmp(argv[i], ">") == 0) {
            int fd = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            argv[i] = NULL;
        }
        else if (strcmp(argv[i], ">>") == 0) {
            int fd = open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(fd, STDOUT_FILENO);
            close(fd);
            argv[i] = NULL;
        }
        else if (strcmp(argv[i], "<") == 0) {
            int fd = open(argv[i+1], O_RDONLY);
            dup2(fd, STDIN_FILENO);
            close(fd);
            argv[i] = NULL;
        }
    }
}

void execute_single(char *cmd) {
    parse_line(cmd, argv);

    if(argv[0] == NULL) return;

    builtin_func bf = find_builtin(argv[0]);
    if (bf) {
        int res = bf(argv);
        if (res == -1) exit(0);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork has failed");
        return;
    }
    if (pid == 0) {
        handle_redirection(argv);
        execvp(argv[0], argv);
        perror("Execution has failed");
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
    Startup();
    while (1) {
        prompter();
        execute_line(comline);
    }
    return 0;
}