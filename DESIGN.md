# Design Decisions

## Zero External Dependencies

Quip links only against libc — no libreadline, no libffi, no JSON parser, no
anything else. This keeps the stripped binary at ~39K and startup under 2ms.
The tradeoff is that every feature (line editing, JSON-over-socket, config
parsing) is hand-rolled C. Adding a dependency would need to justify the
bloat.

## Custom Line Editor vs libreadline

Rather than pull in readline (which adds ~200K and GPL baggage), quip has a
~120-line custom editor in `prompt.c`. It provides insert-mode editing,
arrow-key history navigation, and tab completion. What it doesn't have:
kill-ring, vi-mode, bracketed paste, or multi-byte character support. For a
personal shell this is acceptable; for a general-purpose shell it would not
be.

## Lazy Python Daemon

The plugin daemon forks on first plugin use, not at shell startup. This means
`quip` starts in ~2ms regardless of how many Python plugins are installed.
The cost is a ~100ms delay on the first plugin command while the daemon
starts, connects, and processes the request.

## Abstract Unix Socket for Plugin IPC

The C side communicates with the Python daemon over a Linux abstract Unix
domain socket (`"\0quip-daemon"`). This avoids a filesystem entry (no stale
socket files to clean up) and works without any special permissions. The
downside: it is Linux-only. Porting to macOS or FreeBSD would require
switching to a path-based socket.

## Single Translation Unit Build

All `.c` files are compiled in a single `gcc` invocation:

    gcc -Wall -Wextra -O2 -std=c17 -o quip src/*.c

This disables incremental compilation but keeps the Makefile trivial (25
lines) and enables the compiler to inline across translation units without
LTO.

## Fork-Exec Model

External commands are spawned with `fork()` + `execvp()`. `posix_spawn()` was
considered but `fork()` gives direct control over signal dispositions and fd
manipulation in the child, which matters for pipeline plumbing and
redirection. The cost is a double address-space copy on each command — even
with COW, `fork()` is heavier than `posix_spawn()` on some kernels.

## Builtin / Plugin Redirection via fd Save-Restore

Builtins (and plugins) run in the shell process, so I/O redirection must be
applied and then undone around the call. The approach:

1. `fflush(stdout)` / `fflush(stderr)` — drain stdio buffers so `dup2`
   doesn't strand data
2. `dup()` each standard fd — save a copy of the originals
3. `handle_redirection(args)` — `dup2()` to files/pipes, strip operators from
   argv
4. Execute the builtin — output goes to the redirected fd
5. `fflush()` again, `dup2()` saved fds back, `close()` saved copies

This is more work than the fork+exec path (where the child simply exits and
the fds are naturally reclaimed) but it means `echo hello > file` works
without spawning a process.

## Multiline via Backslash and Quote Tracking

The Enter key normally terminates input. Before doing so, `read_line()`
checks:

- **Trailing backslash**: an odd number of `\` characters at the end of the
  buffer signals line continuation; one `\` is consumed.
- **Unmatched double-quotes**: an odd count of `"` means a string is open.

Either condition prints a `──>` continuation prompt and reads more input.
This is simpler than implementing a full parser state machine (which would
need to track heredocs, `$()` nesting, etc.) and covers the common cases for
daily interactive use.

## No Job Control Signals

`SIGTSTP` (Ctrl-Z) is ignored. Backgrounding is done exclusively via the `&`
syntax and `bg`/`fg` builtins. This avoids the complexity of terminal
process-group management and keeps the signal handler trivial. The tradeoff
is that Ctrl-Z does nothing — users coming from bash must use `&` explicitly.

## Color Scheme

Colors are compile-time constants in `quip.h`. The palette uses xterm 256-color
codes:

| Token       | ANSI Code      | Color |
|-------------|----------------|-------|
| `ACCENT`    | `38;5;214`     | Orange |
| `GRAY`      | `38;5;255`     | Bright white |
| `DIM`       | `38;5;250`     | Light gray |
| `RED`       | `38;5;196`     | Red |
| `BOLD`      | `1`            | Bold weight |

There is no runtime theme support. The `NO_COLOR` environment variable or
a `--no-color` flag would be natural additions for users who want to disable
ANSI output completely.
