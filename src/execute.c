#include "quip.h"

int parse_command_line(char *line, char **argv) {
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
                if (*p == '\\' && *(p+1)) {
                    memmove(p, p+1, strlen(p));
                } else {
                    p++;
                }
            }
            if (*p) *p++ = '\0';
        }
    }

    argv[argc] = NULL;
    return argc;
}

void handle_redirection(char **argv) {
    if (!argv) return;

    int i = 0;
    while (argv[i] != NULL) {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], ">>") == 0) {
            if (argv[i+1] == NULL) {
                fprintf(stderr, ANSI_RED "quip: missing filename after '%s'\n" ANSI_RESET, argv[i]);
                i++;
                continue;
            }

            int flags = O_WRONLY | O_CREAT;
            flags |= (argv[i][1] == '>') ? O_APPEND : O_TRUNC;

            int fd = open(argv[i+1], flags, 0644);
            if (fd < 0) {
                fprintf(stderr, ANSI_RED "open failed: %s\n" ANSI_RESET, strerror(errno));
                i++;
                continue;
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                fprintf(stderr, ANSI_RED "dup2 failed: %s\n" ANSI_RESET, strerror(errno));
                close(fd);
                i++;
                continue;
            }

            close(fd);
            int j = i + 2;
            while (argv[j] != NULL) j++;
            memmove(&argv[i], &argv[i+2], (j - i + 1) * sizeof(char *));

        } else if (strcmp(argv[i], "<") == 0) {
            if (argv[i+1] == NULL) {
                fprintf(stderr, ANSI_RED "quip: missing filename after '<'\n" ANSI_RESET);
                i++;
                continue;
            }

            int fd = open(argv[i+1], O_RDONLY);
            if (fd < 0) {
                fprintf(stderr, ANSI_RED "open failed: %s\n" ANSI_RESET, strerror(errno));
                i++;
                continue;
            }

            if (dup2(fd, STDIN_FILENO) == -1) {
                fprintf(stderr, ANSI_RED "dup2 failed: %s\n" ANSI_RESET, strerror(errno));
                close(fd);
                i++;
                continue;
            }

            close(fd);
            int j = i + 2;
            while (argv[j] != NULL) j++;
            memmove(&argv[i], &argv[i+2], (j - i + 1) * sizeof(char *));

        } else {
            i++;
        }
    }
}

void execute_command(char *cmd) {
    if (!cmd || !*cmd) return;

    char *args[MAX_ARGS];

    int is_background = is_background_command(cmd);
    char cmd_copy[MAX_LINE];
    strncpy(cmd_copy, cmd, MAX_LINE - 1);
    cmd_copy[MAX_LINE - 1] = '\0';

    if (is_background)
        strip_background_ampersand(cmd_copy);

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

    if (plugin_exec(args) == 0) {
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, ANSI_RED "fork failed: %s\n" ANSI_RESET, strerror(errno));
        return;
    }

    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        handle_redirection(args);
        execvp(args[0], args);
        fprintf(stderr, ANSI_RED "%s: command not found\n" ANSI_RESET, args[0]);
        exit(EXIT_FAILURE);
    } else {
        if (is_background) {
            int job_id = add_job(pid, cmd_copy);
            printf(ANSI_GREEN "[%d] %d\n" ANSI_RESET, job_id + 1, pid);
        } else {
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                fprintf(stderr, ANSI_RED "waitpid failed: %s\n" ANSI_RESET, strerror(errno));
            }
        }
    }
}

void execute_line(char *line) {
    if (!line) return;

    char *line_copy = strdup(line);
    if (!line_copy) {
        fprintf(stderr, ANSI_RED "memory allocation failed\n" ANSI_RESET);
        return;
    }

    if (strchr(line_copy, '|')) {
        char *commands[MAX_ARGS];
        int cmd_count = parse_pipeline(line_copy, commands);
        if (cmd_count > 1) {
            execute_pipeline(commands);
        } else {
            execute_command(commands[0]);
        }

        for (int i = 0; i < cmd_count; i++) {
            free(commands[i]);
        }
    } else {
        char *saveptr;
        char *cmd = strtok_r(line_copy, ";", &saveptr);
        while (cmd != NULL) {
            execute_command(cmd);
            cmd = strtok_r(NULL, ";", &saveptr);
        }
    }

    free(line_copy);
}

int parse_pipeline(char *line, char **commands) {
    if (!line || !commands) return 0;

    int count = 0;
    char *saveptr;
    char *token = strtok_r(line, "|", &saveptr);

    while (token != NULL && count < MAX_ARGS - 1) {
        while (*token == ' ' || *token == '\t') token++;

        if (*token != '\0') {
            char *end = token + strlen(token) - 1;
            while (end > token && (*end == ' ' || *end == '\t')) end--;
            *(end + 1) = '\0';

            commands[count] = strdup(token);
            if (commands[count] == NULL) {
                for (int i = 0; i < count; i++) free(commands[i]);
                return 0;
            }
            count++;
        }

        token = strtok_r(NULL, "|", &saveptr);
    }

    commands[count] = NULL;
    return count;
}

static void cleanup_pipes(int nfds, pid_t *pids, int rfd, int wfd, int infd) {
    if (rfd >= 0 && rfd != STDIN_FILENO) close(rfd);
    if (wfd >= 0 && wfd != STDOUT_FILENO) close(wfd);
    if (infd >= 0 && infd != STDIN_FILENO && infd != STDOUT_FILENO) close(infd);

    for (int i = 0; i < nfds; i++) {
        if (pids[i] > 0) waitpid(pids[i], NULL, 0);
    }
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
    pid_t pids[MAX_ARGS];
    int num_forked = 0;
    int in_fd = STDIN_FILENO;
    int last_pipe_read = -1;
    int last_pipe_write = -1;

    for (int i = 0; i < num_commands; i++) {
        last_pipe_read = -1;
        last_pipe_write = -1;

        if (i < num_commands - 1) {
            if (pipe(pipefd) == -1) {
                fprintf(stderr, ANSI_RED "pipe failed: %s\n" ANSI_RESET, strerror(errno));
                cleanup_pipes(num_forked, pids, -1, -1, in_fd);
                return;
            }
            last_pipe_read = pipefd[0];
            last_pipe_write = pipefd[1];
        }

        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, ANSI_RED "fork failed: %s\n" ANSI_RESET, strerror(errno));
            cleanup_pipes(num_forked, pids,
                            last_pipe_read, last_pipe_write, in_fd);
            return;
        }

        pids[num_forked++] = pid;

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            signal(SIGTERM, SIG_DFL);

            if (i > 0) {
                if (dup2(in_fd, STDIN_FILENO) == -1)
                    _exit(127);
                close(in_fd);
            }

            if (i < num_commands - 1) {
                close(pipefd[0]);
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                    _exit(127);
                close(pipefd[1]);
            }

            char *cmd_args[MAX_ARGS];
            int argc = parse_command_line(commands[i], cmd_args);

            if (argc == 0 || cmd_args[0] == NULL)
                exit(EXIT_FAILURE);

            builtin_func bf = find_builtin(cmd_args[0]);
            if (bf) {
                int result = bf(cmd_args);
                exit(result == 1 ? 0 : 1);
            }

            handle_redirection(cmd_args);
            execvp(cmd_args[0], cmd_args);
            fprintf(stderr, ANSI_RED "%s: command not found\n" ANSI_RESET, cmd_args[0]);
            _exit(EXIT_FAILURE);
        } else {
            if (i > 0 && in_fd != STDIN_FILENO) {
                close(in_fd);
            }

            if (i < num_commands - 1) {
                close(pipefd[1]);
                in_fd = pipefd[0];
            }
        }
    }

    int status;
    for (int i = 0; i < num_forked; i++) {
        waitpid(pids[i], &status, 0);
    }

    if (in_fd != STDIN_FILENO) {
        close(in_fd);
    }
}
