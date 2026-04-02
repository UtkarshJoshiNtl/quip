#include "quip.h"

static char *args[MAX_ARGS];

int parse_command_line(char *line, char **argv) {
    if (!line || !argv) return 0;
    
    int argc = 0;
    char *p = line;

    while (*p && argc < MAX_ARGS - 1) {
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
    if (!argv) return;
    
    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            if (argv[i+1] == NULL) {
                fprintf(stderr, "quip: missing filename after '%s'\n", argv[i]);
                continue;
            }
            
            int flags = O_WRONLY | O_CREAT;
            flags |= (argv[i][1] == '>') ? O_APPEND : O_TRUNC;
            
            int fd = open(argv[i+1], flags, 0644);
            if (fd < 0) {
                perror("open failed");
                continue;
            }
            
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2 failed");
                close(fd);
                continue;
            }
            
            close(fd);
            argv[i] = NULL;
            argv[i+1] = NULL;
        } else if (strcmp(argv[i], "<") == 0) {
            if (argv[i+1] == NULL) {
                fprintf(stderr, "quip: missing filename after '<'\n");
                continue;
            }
            
            int fd = open(argv[i+1], O_RDONLY);
            if (fd < 0) {
                perror("open failed");
                continue;
            }
            
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2 failed");
                close(fd);
                continue;
            }
            
            close(fd);
            argv[i] = NULL;
            argv[i+1] = NULL;
        }
    }
}

void execute_command(char *cmd) {
    if (!cmd || !*cmd) return;
    
    int is_background = is_background_command(cmd);
    char cmd_copy[MAX_LINE];
    strncpy(cmd_copy, cmd, MAX_LINE - 1);
    cmd_copy[MAX_LINE - 1] = '\0';
    
    if (is_background) {
        strip_background_ampersand(cmd_copy);
    }
    
    int argc = parse_command_line(cmd_copy, args);
    if (argc == 0 || args[0] == NULL) return;

    builtin_func bf = find_builtin(args[0]);
    if (bf) {
        int result = bf(args);
        if (result == -1) {
            exit(0);
        }
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }
    
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        
        handle_redirection(args);
        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {
        if (is_background) {
            int job_id = add_job(pid, cmd_copy);
            printf("[%d] %d\n", job_id + 1, pid);
        } else {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid failed");
            }
        }
    }
}

void execute_line(char *line) {
    if (!line) return;
    
    char *line_copy = strdup(line);
    if (!line_copy) {
        perror("strdup failed");
        return;
    }
    
    char *cmd = strtok(line_copy, ";");
    while (cmd != NULL) {
        execute_command(cmd);
        cmd = strtok(NULL, ";");
    }
    
    free(line_copy);
}
