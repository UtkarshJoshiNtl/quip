# Architecture

## Overview

Quip follows a classic REPL (Read-Eval-Print-Loop) structure. The main loop
lives in `main.c` and coordinates four subsystems: terminal, history, plugins,
and job control.

```
main.c
  |
  |-- init: terminal, signals, history, jobs, prompt, plugin
  |
  |-- loop:
  |     read_line()       -- prompt.c (read)
  |     execute_line()    -- execute.c (eval)
  |       |
  |       |-- builtin ── builtins.c
  |       |-- plugin  ── plugin.c + quip_daemon.py
  |       |-- fork+exec ── execvp()
  |
  |-- cleanup: prompt, plugin, terminal, history, signals
```

## Module Dependency Graph

```
main.c
  ├── config.c       (no deps)
  ├── signals.c      (no deps)
  ├── jobs.c         (no deps)
  ├── history.c      (config.c)
  ├── terminal.c     (no deps)
  ├── prompt.c       (history.c, builtins.c for tab completion)
  ├── execute.c      (builtins.c, plugin.c, jobs.c)
  ├── builtins.c     (jobs.c)
  ├── plugin.c       (config.c)
  ├── completion.c   (quip.h only)
  └── quip.h         (shared declarations)
```

`quip.h` is the single header. It defines ANSI color macros, constants
(`MAX_LINE`, `MAX_ARGS`, `MAX_HISTORY`), and all function prototypes.

## Command Dispatch Chain

`execute_command()` in `execute.c` dispatches every command through three
stages, in order:

```
        args
          |
          v
    find_builtin() ──yes──> builtin_func(args)
          |
          no
          |
          v
    plugin_exec() ──yes──>  (daemon runs plugin, returns output)
          |
          no
          |
          v
    fork()
      ├── child: handle_redirection() → execvp()
      └── parent: waitpid() / add_job()
```

Redirection (`handle_redirection()`) is applied at each stage:
- **Builtins**: fds saved → `dup2()` to files → builtin runs → fds restored.
- **Plugins**: same save/restore pattern.
- **External**: applied in the child after `fork()`, before `execvp()`. No
  restore needed — the child exits.

## Pipeline Implementation

`execute_pipeline()` in `execute.c`:

```
echo hello | tr a-z A-Z | wc -c
```

1. `parse_pipeline()` splits on `|`, returning N command strings.
2. For each command `i` (0 .. N-1):
   - If not last: `pipe()` creates a new channel.
   - `fork()` a child.
   - Child: `dup2()` read-end from previous pipe (if `i > 0`), `dup2()`
     write-end to current pipe (if `i < N-1`), then
     `handle_redirection()` + `find_builtin()` / `execvp()`.
   - Parent: closes its copy of the pipe ends, records the PID.
3. Parent waits for all children via `waitpid()`.

This means builtins in the middle of a pipeline (`echo hello | cat`) run in
a child process, with their stdin/stdout connected to pipes. Redirection
is applied after pipe setup, so `echo hello | cat > file` works correctly.

## Plugin IPC Protocol

The C side (`plugin.c`) and Python daemon (`quip_daemon.py`) communicate
over a Unix domain socket with length-prefixed JSON messages:

```
[4 bytes: payload length (network byte order)]
[N bytes: UTF-8 JSON payload]
```

### Message Types

| Type | Direction | Purpose |
|------|-----------|---------|
| `exec` | C → Python | Execute a plugin command |
| `result` | Python → C | Return stdout/stderr/exit code |
| `reload` | C → Python | Re-scan plugin directories |
| `shutdown` | C → Python | Graceful daemon shutdown |
| `ok` | Python → C | Acknowledge reload/shutdown |
| `error` | Python → C | Error details |

### Exec Flow

```
C                          Python
|                            |
|-- {type:"exec",            |
|    plugin:"hello",         |
|    argv:["hello","quip"],  |
|    cwd:"/home/user"}       |
|--------------------------->|
|                            |-- import hello.py
|                            |-- hello.execute(argv, cwd, env)
|                            |
|<- {type:"result",          |
|    stdout:"Hello, quip!",  |
|    stderr:"",              |
|    exit_code:0}            |
```

The daemon is forked lazily from `plugin_launch_daemon()` and binds to the
abstract socket `"\0quip-daemon"`. If the daemon crashes, `daemon_launched`
resets to 0 and the next plugin command re-launches it.

## Data Flow Example

Command: `echo hello | wc -c > out`

```
main.c: read_line() ── "echo hello | wc -c > out"
                          |
execute_line()             |
  ├─ strchr('|') found    |
  │   └─ parse_pipeline() ── ["echo hello", "wc -c > out"]
  │       └─ execute_pipeline()
  │           ├─ fork child 0
  │           │   └─ parse "echo hello" → ["echo","hello"]
  │           │   └─ pipe: stdout → pipefd[1]
  │           │   └─ handle_redirection() → no-op
  │           │   └─ find_builtin("echo") → builtin_echo()
  │           │   └─ writes "hello\n" to pipefd[1]
  │           │   └─ exit(0)
  │           │
  │           ├─ fork child 1
  │           │   └─ parse "wc -c > out" → ["wc","-c",">","out"]
  │           │   └─ pipe: stdin → pipefd[0]
  │           │   └─ handle_redirection()
  │           │       └─ open("out", O_WRONLY|O_CREAT|O_TRUNC)
  │           │       └─ dup2(fd, STDOUT_FILENO)
  │           │       └─ argv → ["wc","-c"]
  │           │   └─ find_builtin("wc") → NULL
  │           │   └─ execvp("wc", ["wc","-c"])
  │           │       └─ wc reads "hello\n" from stdin
  │           │       └─ wc writes "6\n" to stdout → file "out"
  │           │
  │           └─ parent: waitpid(child0), waitpid(child1)
```

## Signal Handling

| Signal | Action |
|--------|--------|
| `SIGINT` | Sets `sigint_received = 1`; main loop re-prints prompt on next iteration. Child processes reset to `SIG_DFL` before exec. |
| `SIGTERM` | Sets `should_exit = 1`; loop exits on next iteration. |
| `SIGQUIT` | Ignored (`SIG_IGN`). |
| `SIGTSTP` | Ignored. Job control via `&` / `fg` / `bg` only. |
| `SIGCHLD` | `SIG_DFL` (default). Reaped by explicit `waitpid()` calls in the parent. |

The signal handler is deliberately minimal — it writes `^C\n` to stdout and
sets an atomic flag. All real work (cleanup, re-drawing the prompt) happens
in the main loop.
