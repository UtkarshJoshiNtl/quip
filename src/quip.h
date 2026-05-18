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
#define MAX_HISTORY   128

#define ANSI_RED     "\033[38;5;196m"
#define ANSI_GREEN   "\033[38;5;46m"
#define ANSI_YELLOW  "\033[38;5;220m"
#define ANSI_BLUE    "\033[38;5;51m"
#define ANSI_CYAN    "\033[38;5;141m"
#define ANSI_GRAY    "\033[38;5;240m"
#define ANSI_WHITE   "\033[38;5;255m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_RESET   "\033[0m"

#define CONFIG_MAX_ENTRIES 32
#define CONFIG_KEY_LEN     64
#define CONFIG_VAL_LEN    256

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
int parse_pipeline(char *line, char **commands);
void execute_pipeline(char **commands);

// Built-in commands
int builtin_cd(char **argv);
int builtin_pwd(char **argv);
int builtin_help(char **argv);
int builtin_history(char **argv);
int builtin_exit(char **argv);
int builtin_clear(char **argv);
int builtin_echo(char **argv);
int builtin_env(char **argv);

// Job control
void jobs_init(void);
void cleanup_jobs(void);
int add_job(pid_t pid, const char *command);
void remove_job(int job_id);
int builtin_jobs(char **argv);
int builtin_fg(char **argv);
int builtin_bg(char **argv);
int is_background_command(const char *cmd);
char *strip_background_ampersand(char *cmd);

// Command completion
void handle_completion(char *buf, int *pos, int *len);

// Signal handling
void signals_init(void);
void signals_cleanup(void);
int should_exit_shell(void);
int was_sigint_received(void);

// Configuration
void config_init(void);
const char *config_get(const char *key);
int config_get_int(const char *key, int default_val);
const char *get_data_path(void);
const char *get_config_path(void);

// Plugin system
void plugin_init(void);
int plugin_available(void);
int plugin_exec(char **argv);
void plugin_cleanup(void);

// Utility functions
char *get_command_line(void);
void handle_redirection(char **argv);

#endif // QUIP_H
