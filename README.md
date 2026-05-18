# Quip Shell

A lightweight Unix shell with a C core and Python plugin system.

## Features

- **Interactive & Non-interactive Mode**: Works both as an interactive shell and for scripting
- **Command History**: Persisted to disk via XDG path (`~/.local/share/quip/history`), configurable size
- **Job Control**: Background processes with `&`, `jobs`, `fg`, `bg` commands
- **Pipelines**: Support for piping multiple commands (`ls | grep foo | wc -l`)
- **I/O Redirection**: Support for `<`, `>`, `>>` operators
- **Tab Completion**: Command and filename completion with colorized output
- **Python Plugin System**: Extend the shell with Python scripts — drop a `.py` file in `~/.config/quip/plugins/` and it becomes a command
- **Config File**: INI-style config at `~/.config/quip/config`
- **Colored Output**: Consistent ANSI color scheme for errors, prompts, completions, and jobs
- **Built-in Commands**: Essential shell utilities
- **Memory Safety**: Bounds checking, no leaks (verified), proper cleanup

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
- Python 3 (optional, for plugin system)

## Building

```bash
# Build release version
make

# Build with debug symbols
make debug

# Clean build artifacts
make clean

# Run tests
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
# Pipe commands through shell
echo "ls -la | grep foo" | ./quip

# Multiple commands
printf "cd /tmp\npwd\nexit\n" | ./quip
```

## Plugins

Drop a Python file in `~/.config/quip/plugins/`. Each plugin defines two functions:

```python
def register():
    return {"name": "mycommand", "description": "Does something"}

def execute(argv, cwd, env):
    # argv[0] is the command name
    return (stdout_string, stderr_string, exit_code)
```

### Example: `~/.config/quip/plugins/hello.py`
```python
def register():
    return {"name": "hello", "description": "Prints a greeting"}

def execute(argv, cwd, env):
    name = argv[1] if len(argv) > 1 else "world"
    return (f"Hello, {name}!\n", "", 0)
```

Then use it:
```
> hello quip
Hello, quip!
```

Plugins run in a Python daemon process, communicating with the C core over a Unix domain socket. The daemon is launched lazily on first use — shell startup stays fast.

## Configuration

`~/.config/quip/config` (INI-style):

```ini
history_size = 256
```

| Key | Default | Description |
|-----|---------|-------------|
| `history_size` | 128 | Max history entries (16-4096) |

## Architecture

| File | Description |
|------|-------------|
| `main.c` | Main program loop, initialization, cleanup |
| `config.c` | XDG path resolution, INI config parser |
| `history.c` | Command history (circular buffer, disk-persisted) |
| `terminal.c` | Terminal mode handling (raw/canonical) |
| `prompt.c` | User input, prompt display, ANSI colors |
| `builtins.c` | Built-in command implementations |
| `execute.c` | Command parsing, execution, pipelines, redirection |
| `signals.c` | Signal handling (SIGINT, SIGTERM) |
| `jobs.c` | Job control system (background/foreground) |
| `completion.c` | Tab completion (commands and filenames) |
| `plugin.c` | Python plugin client over Unix domain socket |
| `quip.h` | Header file with declarations and constants |
| `python/quip_daemon.py` | Python daemon: plugin discovery, execution |

## Code Quality

- **Memory Safety**: Bounds checking, circular buffer with proper cleanup
- **Error Handling**: Colored error messages with system error details
- **ANSI Macros**: Consistent color scheme via `quip.h` macros
- **Standards Compliance**: C17 with strict compiler warnings (`-Wall -Wextra`)

## License

MIT License — See repository for details.
