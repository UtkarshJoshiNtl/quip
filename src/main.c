#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char comline[256];
char *argv[64];
void Startup(){
    printf("Starting quip 0.1...\n");
//    system("fastfetch");
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
    (void)argv;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd failed");
    }
    return 1;
}

int builtin_help(char **argv) {
    (void)argv;
    printf("Not implemented yet.\n");
    return 1;
}

int builtin_exit(char **argv) {
    (void)argv;
    return -1; // signal to exit main loop
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

int main() {
    Startup();
    while (1) {
        prompter();
            //tokenize input
            int i = 0;
            char *token = strtok(comline, " \n");
            while (token != NULL)
            {
            argv[i++] = token;
            token = strtok(NULL, " \n");
            }
            argv[i] = NULL;
        if(argv[0] == NULL)
        {
            continue;
        }

        // Built-in command dispatch table
        builtin_func bf = find_builtin(argv[0]);
        if (bf) {
            int res = bf(argv);
            if (res == -1) {
                break;
            }
            continue;
        }

        // External commands
        {
            pid_t pid = fork();
            if (pid < 0) {
                perror("Fork has failed");
                continue;
            }
            if (pid == 0) {
                // Child process get here
                execvp(argv[0], argv);
                perror("Execution has failed");
                exit(EXIT_FAILURE);
            } else {
                // Parent process are here
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }
    return 0;
}