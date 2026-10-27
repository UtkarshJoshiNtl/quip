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
#include <errno.h>

#define MAX_LINE      4096
#define MAX_ARGS       64

#define ANSI_ACCENT  "\033[38;5;214m"
#define ANSI_GRAY    "\033[38;5;255m"
#define ANSI_DIM     "\033[38;5;250m"
#define ANSI_RED     "\033[38;5;196m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_RESET   "\033[0m"

typedef int (*builtin_func)(char **argv);

struct builtin {
    const char *name;
    builtin_func func;
};

void terminal_init(void);
void terminal_cleanup(void);

void prompt_init(void);
void prompt_cleanup(void);
int read_line(char *buffer, size_t size);

int parse_command_line(char *line, char **argv);
void execute_command(char *cmd);
void execute_line(char *line);
builtin_func find_builtin(const char *name);
int parse_pipeline(char *line, char **commands);
void execute_pipeline(char **commands);

int builtin_cd(char **argv);
int builtin_pwd(char **argv);
int builtin_help(char **argv);
int builtin_exit(char **argv);
int builtin_clear(char **argv);
int builtin_echo(char **argv);
int builtin_env(char **argv);

void signals_init(void);
void signals_cleanup(void);
void signal_exit_shell(void);
int should_exit_shell(void);
int was_sigint_received(void);

char *get_command_line(void);
void print_prompt(void);
void handle_redirection(char **argv);

#endif
