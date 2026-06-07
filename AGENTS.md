# quip shell — agent guide

## Build & run

```bash
make          # release build: gcc -Wall -Wextra -Werror -O2 -std=c17 -o quip src/*.c
make debug    # adds -g -DDEBUG
make clean
./quip        # interactive shell
echo "ls" | ./quip  # non-interactive (scripting via stdin)
```

No incremental compilation — single gcc invocation compiles all `.c` files together. Build artifact `quip` is in `.gitignore`.

## No tests

`make test` prints "No tests yet". There is no test framework or CI pipeline (no `.github/`).

## Code structure

All source in `src/`. Single header `src/quip.h` — no other headers. Entrypoint: `src/main.c:main()`.

Three-stage command dispatch (in `src/execute.c`):
```
find_builtin() → plugin_exec() → fork() + execvp()
```

## Python plugin daemon

`python/quip_daemon.py` is forked lazily by `plugin.c` on first plugin use (not at shell startup). IPC over Linux abstract Unix socket `\0quip-daemon` with length-prefixed JSON messages (4-byte big-endian length + UTF-8 JSON payload).

## Key constraints

- C17 with `-Wall -Wextra -Werror` — must compile cleanly
- Zero external deps (libc only). No libreadline, no libffi, no JSON library
- `_GNU_SOURCE` required for abstract sockets, `termios`, etc.
- Linux-only (abstract sockets are Linux-specific)
- `quip` binary in project root, not `src/`
- `graphify-out/` is generated artifact, not part of the project

## Config

`~/.config/quip/config` (INI-like). Key: `history_size` (default 128, range 16–4096).
