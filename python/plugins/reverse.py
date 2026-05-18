"""Reverses each argument and prints them."""

def register():
    return {
        "name": "reverse",
        "description": "Reverse each argument",
        "version": "1.0.0",
    }

def execute(argv, cwd, env):
    parts = [a[::-1] for a in argv[1:]] if len(argv) > 1 else ["please provide arguments"]
    return (" ".join(parts) + "\n", "", 0)
