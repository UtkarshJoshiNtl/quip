# Quip Shell

A lightweight Unix shell written in C with a Python plugin system.

## Features

- Interactive and non-interactive mode (scripting via stdin)
- Command history persisted to disk
- Job control: background with `&`, `jobs`, `fg`, `bg`
- Pipelines: `ls | grep foo | wc -l`
- I/O redirection: `<`, `>`, `>>`
- Tab completion for commands and files
- Python plugins — drop a `.py` file in `~/.config/quip/plugins/`
- Config file at `~/.config/quip/config`
- Colored prompt and output

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

- GCC
- POSIX system (Linux, macOS, WSL)
- GNU Make
- Python 3 (optional, for plugins)

## Building

```bash
make           # release build
make debug     # with debug symbols
make clean     # remove build artifacts
make test      # run tests
sudo make install  # optional, copies to /usr/local/bin
```

## Usage

```bash
# interactive
./quip

# non-interactive
echo "ls -la | grep foo" | ./quip
printf "cd /tmp\npwd\nexit\n" | ./quip
```

## Plugins

Put a Python file in `~/.config/quip/plugins/`. Each plugin needs two functions:

```python
def register():
    return {"name": "mycommand", "description": "Does something"}

def execute(argv, cwd, env):
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

```
> hello quip
Hello, quip!
```

The Python daemon launches on first plugin use so shell startup stays fast.

## Configuration

`~/.config/quip/config`:

```ini
history_size = 256
```

| Key | Default | Description |
|-----|---------|-------------|
| `history_size` | 128 | Max history entries (16-4096) |

## Architecture

| File | Description |
|------|-------------|
| `main.c` | Entry point, init loop, cleanup |
| `config.c` | XDG paths, config file parser |
| `history.c` | Circular buffer, disk persistence |
| `terminal.c` | Raw terminal mode |
| `prompt.c` | Prompt rendering, line editing |
| `builtins.c` | Built-in commands |
| `execute.c` | Parsing, execution, pipes, redirection |
| `signals.c` | SIGINT/SIGTERM handling |
| `jobs.c` | Background job tracking |
| `completion.c` | Tab completion |
| `plugin.c` | Plugin daemon communication |
| `quip.h` | Shared declarations |
| `python/quip_daemon.py` | Python plugin daemon |

## Metrics

| Metric | Value |
|--------|-------|
| Compile time | ~0.9s (gcc -O2) |
| Binary size (stripped) | 39K |
| Startup time | ~2ms |
| Memory usage | ~1.8 MB RSS |
| Total lines | 1973 (1738 C, 235 Python) |

### Test Results

| Test | Status |
|------|--------|
| Builtins (cd, pwd, echo, exit, etc.) | Pass |
| Pipelines (ls \| grep \| wc) | Pass |
| Semicolon chaining | Pass |
| Background jobs | Pass |
| I/O redirection | Pass |
| Tab completion | Pass |

## License

MIT
