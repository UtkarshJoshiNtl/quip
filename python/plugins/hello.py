def register():
    return {
        "name": "hello",
        "description": "Prints a greeting",
    }

def execute(argv, cwd, env):
    name = argv[1] if len(argv) > 1 else "world"
    return (f"Hello, {name}!\n", "", 0)
