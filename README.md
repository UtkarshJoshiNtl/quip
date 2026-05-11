# Quip Shell

A lightweight Unix shell implementation written in C with modern features.

## Features

- **Interactive & Non-interactive Mode**: Works both as an interactive shell and for scripting
- **Command History**: Navigate through command history with arrow keys (64 entries max)
- **Job Control**: Background processes with `&`, `jobs`, `fg`, `bg` commands
- **Pipelines**: Support for piping multiple commands (`ls | grep foo | wc -l`)
- **Signal Handling**: Graceful shutdown with Ctrl+C, proper process cleanup
- **Tab Completion**: Command and filename completion
- **I/O Redirection**: Support for `<`, `>`, `>>` operators
- **Built-in Commands**: Essential shell utilities
- **Colored Output**: Modern ANSI-colored welcome banner and prompts
- **Memory Safety**: Proper bounds checking and cleanup

## Built-in Commands

| Command | Description |
|---------|-------------|
| `cd [dir]` | Change directory |
| `pwd` | Print working directory |
| `help` | Show available commands |
| `history` | Show command history |
| `clear` | Clear screen |
| `echo [text]` | Print text |
| `env` | Display environment variables |
| `jobs` | List background jobs |
| `fg [job_id]` | Bring job to foreground |
| `bg [job_id]` | Resume job in background |
| `exit` | Exit shell |

## Prerequisites

- GCC compiler
- POSIX-compliant system (Linux, macOS, WSL)
- GNU Make

## Building

```bash
# Build release version
make

# Build with debug symbols
make debug

# Clean build artifacts
make clean

# Run tests (requires test.sh)
make test

# Install to /usr/local/bin (optional)
sudo make install
```

## Usage

### Interactive Mode
```bash
./quip
```

### Non-interactive Mode
```bash
# Execute single command
./quip -c "ls -la"

# Pipe commands through shell
echo "help" | ./quip
echo -e "cd /tmp\nls\nexit" | ./quip
```

### Examples

```bash
# Pipelines
ls -la /usr/bin | grep python | wc -l
cat file.txt | sort | uniq | head -20

# I/O Redirection
echo "Hello World" > output.txt
cat < input.txt > output.txt
echo "Appended" >> output.txt

# Background Processes
sleep 10 &                # Run in background
jobs                      # List background jobs
fg 1                      # Bring job 1 to foreground
bg 1                      # Resume job 1 in background

# Command History
# Press Up/Down arrows to navigate history
history                   # Display history
!!                        # Repeat last command (if supported)
```

## Architecture

The shell is organized into modular components in `src/`:

| File | Description |
|------|-------------|
| `main.c` | Main program loop, initialization, cleanup |
| `history.c` | Command history management (64 entries, circular buffer) |
| `terminal.c` | Terminal mode handling (raw/canonical) |
| `prompt.c` | User input, prompt display, ANSI colors |
| `builtins.c` | Built-in command implementations |
| `execute.c` | Command parsing, execution, pipelines, redirection |
| `signals.c` | Signal handling (SIGINT, SIGCHLD) |
| `jobs.c` | Job control system (background/foreground) |
| `completion.c` | Tab completion (commands and filenames) |
| `quip.h` | Header file with declarations and constants |

## Configuration

Constants defined in `quip.h`:
- `MAX_LINE`: 4096 bytes (maximum command line length)
- `MAX_ARGS`: 64 (maximum arguments per command)
- `MAX_HISTORY`: 64 (maximum history entries)

## Code Quality

- **Memory Safety**: Proper bounds checking and memory management
- **Error Handling**: Comprehensive error checking for system calls
- **Modular Design**: Clean separation of concerns
- **Standards Compliance**: C99 compliant with strict compiler warnings (`-Wall -Wextra`)

## License

MIT License - See repository for details.
