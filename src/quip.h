#ifndef QUIP_H
#define QUIP_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_LINE    256
#define MAX_ARGS     64
#define MAX_HISTORY  64

typedef int (*builtin_func)(char **argv);

struct builtin {
    const char *name;
    builtin_func func;
};

// History management
void history_init(void);
void history_cleanup(void);
void history_push(const char *line);
const char *history_get(int index);
int history_size(void);

// Terminal management
void terminal_init(void);
void terminal_cleanup(void);

// Prompt and input
void prompt_init(void);
void prompt_cleanup(void);
int read_line(char *buffer, size_t size);

// Command parsing and execution
int parse_command_line(char *line, char **argv);
void execute_command(char *cmd);
void execute_line(char *line);
builtin_func find_builtin(const char *name);

// Built-in commands
int builtin_cd(char **argv);
int builtin_pwd(char **argv);
int builtin_help(char **argv);
int builtin_history(char **argv);
int builtin_exit(char **argv);
int builtin_clear(char **argv);
int builtin_echo(char **argv);
int builtin_env(char **argv);

// Utility functions
char *get_command_line(void);
void handle_redirection(char **argv);

#endif // QUIP_H
