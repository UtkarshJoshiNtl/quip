#include "quip.h"

int builtin_cd(char **argv) {
    if (argv[1] != NULL) {
        if (chdir(argv[1]) != 0) {
            fprintf(stderr, ANSI_RED "cd: %s: %s\n" ANSI_RESET, argv[1], strerror(errno));
            return 0;
        }
    } else {
        char *home = getenv("HOME");
        if (home) {
            if (chdir(home) != 0) {
                fprintf(stderr, ANSI_RED "cd: %s: %s\n" ANSI_RESET, home, strerror(errno));
                return 0;
            }
        } else {
            fprintf(stderr, ANSI_RED "cd: HOME not set and no directory specified\n" ANSI_RESET);
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
    printf(ANSI_GREEN "╭─────────────────────────────────────────────────────╮\n");
    printf("│" ANSI_YELLOW "              Quip Shell v0.4 - Help              " ANSI_GREEN "│\n");
    printf("╰─────────────────────────────────────────────────────╯\n");
    printf(ANSI_WHITE "Built-in Commands:\n" ANSI_RESET);
    printf(ANSI_GRAY "├─" ANSI_BLUE " cd [dir]      " ANSI_WHITE "- Change directory\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " pwd           " ANSI_WHITE "- Print working directory\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " help          " ANSI_WHITE "- Show this help message\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " history       " ANSI_WHITE "- Show command history\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " clear         " ANSI_WHITE "- Clear the screen\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " echo [text]   " ANSI_WHITE "- Print text to stdout\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " env           " ANSI_WHITE "- List environment variables\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " jobs          " ANSI_WHITE "- List background jobs\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " fg [job_id]   " ANSI_WHITE "- Bring job to foreground\n");
    printf(ANSI_GRAY "├─" ANSI_BLUE " bg [job_id]   " ANSI_WHITE "- Resume job in background\n");
    printf(ANSI_GRAY "└─" ANSI_BLUE " exit          " ANSI_WHITE "- Exit the shell\n");
    printf("\n" ANSI_YELLOW "Special Features:\n" ANSI_RESET);
    printf(ANSI_GRAY "├─" ANSI_GREEN " Pipes         " ANSI_WHITE "- Use | to pipe commands\n");
    printf(ANSI_GRAY "├─" ANSI_GREEN " Redirection   " ANSI_WHITE "- Use >, >>, < for I/O redirection\n");
    printf(ANSI_GRAY "├─" ANSI_GREEN " Background    " ANSI_WHITE "- Use & to run commands in background\n");
    printf(ANSI_GRAY "├─" ANSI_GREEN " History       " ANSI_WHITE "- Use up/down arrows to navigate history\n");
    printf(ANSI_GRAY "├─" ANSI_GREEN " Tab complete  " ANSI_WHITE "- Use TAB to complete commands/files\n");
    printf(ANSI_GRAY "├─" ANSI_GREEN " Plugins       " ANSI_WHITE "- Extend with Python scripts in ~/.config/quip/plugins/\n");
    printf(ANSI_GRAY "└─" ANSI_GREEN " Ctrl+C        " ANSI_WHITE "- Cancel current command\n" ANSI_RESET);
    return 1;
}

int builtin_history(char **argv) {
    (void)argv;
    for (int i = 0; i < history_size(); i++) {
        const char *cmd = history_get(i);
        if (cmd) {
            printf(ANSI_GRAY "  %3d  " ANSI_WHITE "%s\n" ANSI_RESET, i + 1, cmd);
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
