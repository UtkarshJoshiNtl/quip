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
    printf("\033[38;5;46m╭─────────────────────────────────────────────────────╮\033[0m\n");
    printf("\033[38;5;46m│\033[38;5;220m              Quip Shell v0.3 - Help              \033[38;5;46m│\033[0m\n");
    printf("\033[38;5;46m╰─────────────────────────────────────────────────────╯\033[0m\n");
    printf("\033[38;5;255m🔧 \033[38;5;220mBuilt-in Commands:\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m cd [dir]      \033[38;5;255m- Change directory\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m pwd           \033[38;5;255m- Print working directory\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m help          \033[38;5;255m- Show this help message\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m history       \033[38;5;255m- Show command history\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m clear         \033[38;5;255m- Clear the screen\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m echo [text]   \033[38;5;255m- Print text to stdout\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m env           \033[38;5;255m- List environment variables\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m jobs          \033[38;5;255m- List background jobs\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m fg [job_id]   \033[38;5;255m- Bring job to foreground\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;51m bg [job_id]   \033[38;5;255m- Resume job in background\033[0m\n");
    printf("\033[38;5;240m└─\033[38;5;51m exit          \033[38;5;255m- Exit the shell\033[0m\n");
    printf("\n\033[38;5;255m✨ \033[38;5;220mSpecial Features:\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;46m Pipes         \033[38;5;255m- Use | to pipe commands\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;46m Redirection   \033[38;5;255m- Use >, >>, < for I/O redirection\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;46m Background    \033[38;5;255m- Use & to run commands in background\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;46m History       \033[38;5;255m- Use ↑/↓ arrows to navigate history\033[0m\n");
    printf("\033[38;5;240m├─\033[38;5;46m Auto-complete \033[38;5;255m- Use TAB to complete commands/files\033[0m\n");
    printf("\033[38;5;240m└─\033[38;5;46m Ctrl+C        \033[38;5;255m- Cancel current command\033[0m\n");
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
    { "jobs",    builtin_jobs    },
    { "fg",      builtin_fg      },
    { "bg",      builtin_bg      },
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
