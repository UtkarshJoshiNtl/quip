#include "quip.h"

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
    
    static char *args[MAX_ARGS];
    
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
    
    // Check if this is a pipeline
    if (strchr(line_copy, '|')) {
        char *commands[MAX_ARGS];
        int cmd_count = parse_pipeline(line_copy, commands);
        if (cmd_count > 1) {
            execute_pipeline(commands);
        } else {
            execute_command(commands[0]);
        }
        
        // Free pipeline commands
        for (int i = 0; i < cmd_count; i++) {
            free(commands[i]);
        }
    } else {
        char *cmd = strtok(line_copy, ";");
        while (cmd != NULL) {
            execute_command(cmd);
            cmd = strtok(NULL, ";");
        }
    }
    
    free(line_copy);
}

int parse_pipeline(char *line, char **commands) {
    if (!line || !commands) return 0;
    
    int count = 0;
    char *token = strtok(line, "|");
    
    while (token != NULL && count < MAX_ARGS - 1) {
        // Skip leading whitespace
        while (*token == ' ' || *token == '\t') token++;
        
        // Remove trailing whitespace
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\t')) end--;
        *(end + 1) = '\0';
        
        commands[count] = strdup(token);
        count++;
        token = strtok(NULL, "|");
    }
    
    commands[count] = NULL;
    return count;
}

void execute_pipeline(char **commands) {
    if (!commands || !commands[0]) return;
    
    int num_commands = 0;
    while (commands[num_commands]) num_commands++;
    
    if (num_commands <= 1) {
        execute_command(commands[0]);
        return;
    }
    
    int pipefd[2];
    pid_t pid;
    int in_fd = STDIN_FILENO;
    
    for (int i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            // Create pipe for all but the last command
            if (pipe(pipefd) == -1) {
                perror("pipe failed");
                return;
            }
        }
        
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            return;
        }
        
        if (pid == 0) {
            // Child process
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            
            // Set up input
            if (i > 0) {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            
            // Set up output
            if (i < num_commands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }
            
            // Parse and execute command
            static char *cmd_args[MAX_ARGS];
            int argc = parse_command_line(commands[i], cmd_args);
            
            if (argc == 0 || cmd_args[0] == NULL) {
                exit(EXIT_FAILURE);
            }
            
            // Check if it's a builtin
            builtin_func bf = find_builtin(cmd_args[0]);
            if (bf) {
                int result = bf(cmd_args);
                exit(result == 1 ? 0 : 1);
            }
            
            // Execute external command
            handle_redirection(cmd_args);
            execvp(cmd_args[0], cmd_args);
            perror("exec failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            if (i > 0) {
                close(in_fd);
            }
            
            if (i < num_commands - 1) {
                close(pipefd[1]);
                in_fd = pipefd[0];
            }
        }
    }
    
    // Wait for all processes in the pipeline
    int status;
    for (int i = 0; i < num_commands; i++) {
        wait(&status);
    }
    
    if (num_commands > 1) {
        close(in_fd);
    }
}
