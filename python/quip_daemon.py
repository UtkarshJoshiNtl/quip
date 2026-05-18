#!/usr/bin/env python3
"""Quip plugin daemon — receives plugin requests over Unix domain socket."""

import json
import os
import socket
import struct
import sys
import traceback

SOCKET_PATH = "\0quip-daemon"
PLUGIN_DIRS = []


def discover_plugin_dirs():
    xdg_config = os.environ.get("XDG_CONFIG_HOME", "")
    home = os.environ.get("HOME", "")
    dirs = []
    if xdg_config:
        dirs.append(os.path.join(xdg_config, "quip", "plugins"))
    if home:
        dirs.append(os.path.join(home, ".config", "quip", "plugins"))
        dirs.append(os.path.join(home, ".local", "share", "quip", "plugins"))
    return [d for d in dirs if os.path.isdir(d)]


def discover_plugins():
    plugins = {}
    for d in PLUGIN_DIRS:
        for fname in sorted(os.listdir(d)):
            if fname.endswith(".py") and not fname.startswith("_"):
                path = os.path.join(d, fname)
                name = fname[:-3]
                plugins[name] = path
    return plugins


def load_plugin_module(path, name):
    import importlib.util
    spec = importlib.util.spec_from_file_location(f"quip_plugin_{name}", path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def send_msg(conn, data):
    payload = json.dumps(data).encode("utf-8")
    conn.sendall(struct.pack("!I", len(payload)) + payload)


def recv_msg(conn):
    raw = conn.recv(4)
    if not raw or len(raw) < 4:
        return None
    msg_len = struct.unpack("!I", raw)[0]
    data = b""
    while len(data) < msg_len:
        chunk = conn.recv(msg_len - len(data))
        if not chunk:
            return None
        data += chunk
    return json.loads(data.decode("utf-8"))


def handle_exec(conn, msg, plugins):
    name = msg.get("plugin", "")
    argv = msg.get("argv", [name])
    cwd = msg.get("cwd", os.getcwd())
    env = msg.get("env", {})

    if name not in plugins:
        send_msg(conn, {"type": "result", "exit_code": 127,
                        "stdout": "", "stderr": f"plugin '{name}' not found"})
        return

    path = plugins[name]
    try:
        mod = load_plugin_module(path, name)
        if not hasattr(mod, "execute"):
            send_msg(conn, {"type": "result", "exit_code": 1,
                            "stdout": "", "stderr": f"plugin '{name}' has no execute()"})
            return

        old_cwd = os.getcwd()
        old_environ = os.environ.copy()

        try:
            if cwd:
                os.chdir(cwd)
            for k, v in env.items():
                os.environ[k] = v

            result = mod.execute(argv, cwd, env)
        finally:
            os.chdir(old_cwd)
            os.environ.clear()
            os.environ.update(old_environ)

        if result is None:
            result = ("", "", 0)
        if isinstance(result, int):
            result = ("", "", result)
        if isinstance(result, str):
            result = (result, "", 0)
        stdout, stderr, code = result

        send_msg(conn, {"type": "result",
                        "stdout": stdout or "",
                        "stderr": stderr or "",
                        "exit_code": code if code is not None else 0})

    except Exception:
        tb = traceback.format_exc()
        send_msg(conn, {"type": "result", "exit_code": 1,
                        "stdout": "", "stderr": tb})


def handle_list(conn, plugins):
    info = {}
    for name, path in plugins.items():
        info[name] = {
            "path": path,
            "description": _get_description(path, name),
        }
    send_msg(conn, {"type": "plugin_list", "plugins": info})


def _get_description(path, name):
    try:
        mod = load_plugin_module(path, name)
        if hasattr(mod, "register"):
            reg = mod.register()
            if isinstance(reg, dict):
                return reg.get("description", "")
        return ""
    except Exception:
        return ""


def main():
    global PLUGIN_DIRS
    PLUGIN_DIRS = discover_plugin_dirs()
    plugins = discover_plugins()

    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.bind(SOCKET_PATH)
    sock.listen(5)

    while True:
        conn, _ = sock.accept()
        try:
            msg = recv_msg(conn)
            if msg is None:
                conn.close()
                continue

            msg_type = msg.get("type", "")

            if msg_type == "exec":
                handle_exec(conn, msg, plugins)
            elif msg_type == "list":
                handle_list(conn, plugins)
            elif msg_type == "reload":
                plugins = discover_plugins()
                send_msg(conn, {"type": "ok", "count": len(plugins)})
            elif msg_type == "shutdown":
                send_msg(conn, {"type": "ok"})
                conn.close()
                break
            else:
                send_msg(conn, {"type": "error",
                                "message": f"unknown type: {msg_type}"})
        except Exception:
            try:
                send_msg(conn, {"type": "error",
                                "message": traceback.format_exc()})
            except Exception:
                pass
        finally:
            conn.close()

    sock.close()


if __name__ == "__main__":
    main()
