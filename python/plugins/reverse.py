def register():
    return {
        "name": "reverse",
        "description": "Reverse each argument",
    }

def execute(argv, cwd, env):
    parts = [a[::-1] for a in argv[1:]] if len(argv) > 1 else ["please provide arguments"]
    return (" ".join(parts) + "\n", "", 0)
