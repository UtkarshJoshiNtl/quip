#include "quip.h"

int builtin_cd(char **argv) {
    if (argv[1] != NULL) {
        if (chdir(argv[1]) != 0) {
            fprintf(stderr, ANSI_DIM "quip: " ANSI_RED "cd: %s: %s\n" ANSI_RESET, argv[1], strerror(errno));
            return 0;
        }
    } else {
        char *home = getenv("HOME");
        if (home) {
            if (chdir(home) != 0) {
                fprintf(stderr, ANSI_DIM "quip: " ANSI_RED "cd: %s: %s\n" ANSI_RESET, home, strerror(errno));
                return 0;
            }
        } else {
            fprintf(stderr, ANSI_DIM "quip: " ANSI_RED "cd: HOME not set and no directory specified\n" ANSI_RESET);
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
        perror("pwd failed");
        return 0;
    }
}

int builtin_help(char **argv) {
    (void)argv;
    printf(ANSI_DIM "quip " ANSI_ACCENT "v0.4" ANSI_DIM " \u2014 Help\n" ANSI_RESET);
    printf(ANSI_GRAY "Built-ins:\n" ANSI_RESET);
    printf(ANSI_DIM "  cd [dir]      " ANSI_GRAY "- Change directory\n");
    printf(ANSI_DIM "  pwd           " ANSI_GRAY "- Print working directory\n");
    printf(ANSI_DIM "  help          " ANSI_GRAY "- Show this help message\n");
    printf(ANSI_DIM "  history       " ANSI_GRAY "- Show command history\n");
    printf(ANSI_DIM "  clear         " ANSI_GRAY "- Clear the screen\n");
    printf(ANSI_DIM "  echo [text]   " ANSI_GRAY "- Print text to stdout\n");
    printf(ANSI_DIM "  env           " ANSI_GRAY "- List environment variables\n");
    printf(ANSI_DIM "  jobs          " ANSI_GRAY "- List background jobs\n");
    printf(ANSI_DIM "  fg [job_id]   " ANSI_GRAY "- Bring job to foreground\n");
    printf(ANSI_DIM "  bg [job_id]   " ANSI_GRAY "- Resume job in background\n");
    printf(ANSI_DIM "  exit          " ANSI_GRAY "- Exit the shell\n");
    printf("\n" ANSI_GRAY "Features: " ANSI_DIM "Pipes, Redirection, Background, History, Tab completion, Plugins, Ctrl+C\n" ANSI_RESET);
    return 1;
}

int builtin_history(char **argv) {
    (void)argv;
    for (int i = 0; i < history_size(); i++) {
        const char *cmd = history_get(i);
        if (cmd) {
            printf(ANSI_DIM "  %3d  " ANSI_GRAY "%s\n" ANSI_RESET, i + 1, cmd);
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
        if (i > 1) printf(" ");
        printf("%s", argv[i]);
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
    { "jobs",    builtin_jobs    },
    { "fg",      builtin_fg      },
    { "bg",      builtin_bg      },
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
