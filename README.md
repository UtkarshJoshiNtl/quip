# Quip Shell

A lightweight Unix shell implementation written in C with modern features.

## Features

- **Interactive & Non-interactive Mode**: Works both as an interactive shell and for scripting
- **Command History**: Navigate through command history with arrow keys
- **Job Control**: Background processes with `&`, `jobs`, `fg`, `bg` commands
- **Signal Handling**: Graceful shutdown with Ctrl+C, proper process cleanup
- **Tab Completion**: Command and filename completion
- **Built-in Commands**: Essential shell utilities
- **I/O Redirection**: Support for `<`, `>`, `>>` operators

## Built-in Commands

- `cd [dir]` - Change directory
- `pwd` - Print working directory
- `help` - Show available commands
- `history` - Show command history
- `clear` - Clear screen
- `echo [text]` - Print text
- `env` - Display environment variables
- `jobs` - List background jobs
- `fg [job_id]` - Bring job to foreground
- `bg [job_id]` - Resume job in background
- `exit` - Exit shell

## Building

```bash
make          # Build release version
make debug    # Build debug version
make clean    # Clean build artifacts
make test     # Run basic tests
```

## Usage

### Interactive Mode
```bash
./quip
```

### Non-interactive Mode
```bash
./quip -c "ls -la"
echo "help" | ./quip
```

### Background Processes
```bash
sleep 10 &    # Run in background
jobs          # List jobs
fg 1          # Bring job 1 to foreground
bg 1          # Resume job 1 in background
```

## Architecture

The shell is organized into modular components:

- `main.c` - Main program loop
- `history.c` - Command history management
- `terminal.c` - Terminal mode handling
- `prompt.c` - User input and prompt
- `builtins.c` - Built-in command implementations
- `execute.c` - Command parsing and execution
- `signals.c` - Signal handling
- `jobs.c` - Job control system
- `completion.c` - Tab completion
- `quip.h` - Header file with declarations

## Code Quality

- **Memory Safety**: Proper bounds checking and memory management
- **Error Handling**: Comprehensive error checking for system calls
- **Modular Design**: Clean separation of concerns
- **Standards Compliance**: C99 compliant with strict compiler warnings

## License

This project is open source. See the repository for licensing information.
