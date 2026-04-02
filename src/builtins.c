#include "quip.h"

int builtin_cd(char **argv) {
    if (argv[1] != NULL) {
        if (chdir(argv[1]) != 0) {
            perror("chdir failed");
            return 0;
        }
    } else {
        char *home = getenv("HOME");
        if (home) {
            if (chdir(home) != 0) {
                perror("chdir to home failed");
                return 0;
            }
        } else {
            fprintf(stderr, "cd: HOME not set and no directory specified\n");
            return 0;
        }
    }
    return 1;
}

int builtin_pwd(char **argv) {
    (void)argv;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 1;
    } else {
        perror("getcwd failed");
        return 0;
    }
}

int builtin_help(char **argv) {
    (void)argv;
    printf("quip shell - version 0.2\n");
    printf("Available builtins:\n");
    extern struct builtin builtins[];
    for (int i = 0; builtins[i].name != NULL; i++) {
        printf("  %s\n", builtins[i].name);
    }
    return 1;
}

int builtin_history(char **argv) {
    (void)argv;
    for (int i = 0; i < history_size(); i++) {
        const char *cmd = history_get(i);
        if (cmd) {
            printf("  %3d  %s\n", i + 1, cmd);
        }
    }
    return 1;
}

int builtin_exit(char **argv) {
    (void)argv;
    return -1;
}

int builtin_clear(char **argv) {
    (void)argv;
    printf("\033[H\033[J");
    fflush(stdout);
    return 1;
}

int builtin_echo(char **argv) {
    for (int i = 1; argv[i] != NULL; i++) {
        printf("%s%s", argv[i], argv[i+1] ? " " : "");
    }
    printf("\n");
    return 1;
}

int builtin_env(char **argv) {
    (void)argv;
    extern char **environ;
    for (char **env = environ; *env; env++) {
        printf("%s\n", *env);
    }
    return 1;
}

struct builtin builtins[] = {
    { "cd",      builtin_cd      },
    { "pwd",     builtin_pwd     },
    { "help",    builtin_help    },
    { "history", builtin_history },
    { "clear",   builtin_clear   },
    { "echo",    builtin_echo    },
    { "env",     builtin_env     },
    { "exit",    builtin_exit    },
    { NULL,      NULL            }
};

builtin_func find_builtin(const char *name) {
    if (!name) return NULL;
    
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0)
            return builtins[i].func;
    }
    return NULL;
}
