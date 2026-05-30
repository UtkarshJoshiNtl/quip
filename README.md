# Quip Shell

A lightweight Unix shell written in C with a Python plugin system.

## Features

- Interactive and non-interactive mode (scripting via stdin)
- Command history persisted to disk
- Job control: background with `&`, `jobs`, `fg`, `bg`
- Pipelines: `ls | grep foo | wc -l`
- I/O redirection: `<`, `>`, `>>`
- Multiline input: backslash (`\`) continuation and unmatched quote tracking
- Tab completion for commands and files
- Python plugins — drop a `.py` file in `~/.config/quip/plugins/`
- Config file at `~/.config/quip/config`
- Colored prompt and output

## Screenshots

![quip terminal](screenshots/quip.png)

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

## Further Reading

- **[DESIGN.md](DESIGN.md)** — design rationale: why no libreadline, how the
  lazy Python daemon works, how builtin redirection saves/restores file
  descriptors, the backslash-continuation approach, and the fork-exec model.
- **[ARCHITECTURE.md](ARCHITECTURE.md)** — system structure: the REPL loop,
  command dispatch chain (builtin → plugin → fork+exec), pipeline
  implementation with N-child fork/pipe, plugin IPC protocol, signal flow,
  and a worked data-flow example.

### Key Points

| From | Key Takeaway |
|------|--------------|
| DESIGN.md | **Zero deps.** The stripped binary is ~39K and starts in ~2ms — no libreadline, no libffi, no JSON library. |
| DESIGN.md | **Lazy daemon.** The Python plugin process forks on first use, not at shell startup. First plugin call is ~100ms slower; subsequent calls are fast. |
| DESIGN.md | **Builtin redirection.** Unlike bash, quip must save/restore fds around builtins (`echo > file`), since they run in the shell process rather than a child. |
| DESIGN.md | **fork+exec.** External commands use `fork()` + `execvp()` for full control over signals and fds in the child. |
| ARCHITECTURE.md | **Three-stage dispatch.** Every command goes through builtin → plugin → fork+exec in sequence. Redirection is applied at every stage. |
| ARCHITECTURE.md | **Pipeline plumbing.** N children on N sides of `\|`, each with `pipe()` + `dup2()`. Builtins in pipelines run in child processes like external commands. |

## Source Map

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
| Total lines | ~2400 (C + Python + docs) |

### Test Results

| Test | Status |
|------|--------|
| Builtins (cd, pwd, echo, exit, etc.) | Pass |
| Pipelines (ls \| grep \| wc) | Pass |
| Plugins in pipelines | Pass |
| Semicolon chaining | Pass |
| Semicolons + pipelines (`echo a \| cat; echo b`) | Pass |
| Quoted pipes / semicolons (`echo "a\|b"` / `echo "a;b"`) | Pass |
| Plugin I/O redirection (`hello > file`) | Pass |
| Background jobs | Pass |
| I/O redirection | Pass |
| Tab completion | Pass |

## License

MIT
