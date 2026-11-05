#!/usr/bin/env python3
"""Development helper: the same commands on every operating system and shell.

    python scripts/dev.py setup [--recreate]   create .venv, install requirements-dev.txt
    python scripts/dev.py test [pytest args]   run pytest inside .venv
    python scripts/dev.py smoke [--write]      raw JSON-RPC check (scripts/jsonrpc_smoke.py)
    python scripts/dev.py run [--allow-writes] start the server over stdio from .venv
    python scripts/dev.py codex-config         print the [mcp_servers.lab] block for Codex

Use `python` or `python3`, whichever starts Python 3.10 or newer on your machine. Only the
standard library is used here; the packages live in .venv, which this script creates and uses,
so there is no need to activate it.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

MIN_PYTHON = (3, 10)
REPO = Path(__file__).resolve().parent.parent
VENV = REPO / ".venv"


def venv_python() -> Path:
    """Path of the interpreter inside .venv (the venv layout differs between platforms)."""
    if os.name == "nt":
        return VENV / "Scripts" / "python.exe"
    return VENV / "bin" / "python"


def fail(message: str) -> int:
    print(f"dev.py: {message}", file=sys.stderr)
    return 1


def check_python(version: tuple[int, ...], what: str) -> str | None:
    if tuple(version[:2]) < MIN_PYTHON:
        found = ".".join(str(v) for v in version[:3])
        return (f"{what} is Python {found}; the MCP SDK needs {MIN_PYTHON[0]}.{MIN_PYTHON[1]} "
                "or newer. Install a newer Python and run this script with it.")
    return None


def venv_version() -> tuple[int, ...] | None:
    py = venv_python()
    if not py.exists():
        return None
    out = subprocess.run([str(py), "-c", "import sys; print(*sys.version_info[:3])"],
                         capture_output=True, text=True)
    if out.returncode != 0:
        return None
    return tuple(int(part) for part in out.stdout.split())


def require_venv() -> Path | None:
    py = venv_python()
    if not py.exists():
        print("dev.py: .venv not found; run `python scripts/dev.py setup` first.", file=sys.stderr)
        return None
    return py


def run(cmd: list[str], env: dict[str, str] | None = None) -> int:
    print("+", " ".join(cmd), flush=True)
    return subprocess.call(cmd, cwd=REPO, env=env)


def cmd_setup(args: argparse.Namespace) -> int:
    if VENV.exists() and args.recreate:
        print(f"removing {VENV}")
        shutil.rmtree(VENV)
    if VENV.exists():
        existing = venv_version()
        if existing is None:
            return fail(f"{VENV} exists but has no working Python; "
                        "run `python scripts/dev.py setup --recreate`.")
        problem = check_python(existing, "The existing .venv")
        if problem:
            return fail(problem.replace("run this script with it.",
                                        "run `setup --recreate` with it."))
        print(f"using existing {VENV}")
    else:
        problem = check_python(sys.version_info, "This interpreter")
        if problem:
            return fail(problem)
        rc = run([sys.executable, "-m", "venv", str(VENV)])
        if rc != 0:
            return rc
    py = str(venv_python())
    return run([py, "-m", "pip", "install", "-r", "requirements-dev.txt"])


def cmd_test(args: argparse.Namespace) -> int:
    py = require_venv()
    if py is None:
        return 1
    return run([str(py), "-m", "pytest", "-q", *args.pytest_args])


def server_env(allow_writes: bool) -> dict[str, str]:
    env = dict(os.environ)
    if allow_writes:
        env["LAB_ALLOW_WRITES"] = "1"
    return env


def cmd_smoke(args: argparse.Namespace) -> int:
    py = require_venv()
    if py is None:
        return 1
    extra = ["--write"] if args.write else []
    return run([str(py), str(Path("scripts") / "jsonrpc_smoke.py"), *extra],
               env=server_env(args.write))


def cmd_run(args: argparse.Namespace) -> int:
    py = require_venv()
    if py is None:
        return 1
    if args.allow_writes:
        print("LAB_ALLOW_WRITES=1: set_instrument_status is allowed", file=sys.stderr)
    print("server on stdio, stop it with Ctrl+C", file=sys.stderr, flush=True)
    try:
        return subprocess.call([str(py), "-m", "mcp_lab_server"], cwd=REPO,
                               env=server_env(args.allow_writes))
    except KeyboardInterrupt:
        return 0


def toml_str(value: str) -> str:
    # A JSON string is a valid TOML basic string (same escapes for \ and ").
    return json.dumps(value, ensure_ascii=False)


def codex_home() -> Path:
    home = os.environ.get("CODEX_HOME")
    return Path(home) if home else Path.home() / ".codex"


def cmd_codex_config(args: argparse.Namespace) -> int:
    py = venv_python()
    if not py.exists():
        print("# Note: .venv does not exist yet; run `python scripts/dev.py setup` first.")
    print(f"# Paste into {codex_home() / 'config.toml'}")
    print("# (made by scripts/dev.py codex-config from codex/config.example.toml)")
    print()
    print("[mcp_servers.lab]")
    print(f"command = {toml_str(str(py))}")
    print('args = ["-m", "mcp_lab_server"]')
    print(f"cwd = {toml_str(str(REPO))}")
    print(f"env = {{ LAB_STATE_DIR = {toml_str(str(REPO / 'var'))} }}")
    print('enabled_tools = ["list_instruments", "get_results"]')
    print('default_tools_approval_mode = "writes"')
    print("startup_timeout_sec = 20")
    print("tool_timeout_sec = 30")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(prog="python scripts/dev.py", description=__doc__.split("\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("setup", help="create .venv and install requirements-dev.txt")
    p.add_argument("--recreate", action="store_true", help="delete .venv first")
    p.set_defaults(func=cmd_setup)

    p = sub.add_parser("test", help="run pytest inside .venv (extra arguments go to pytest)")
    p.add_argument("pytest_args", nargs=argparse.REMAINDER)
    p.set_defaults(func=cmd_test)

    p = sub.add_parser("smoke", help="raw JSON-RPC check of the server without Codex")
    p.add_argument("--write", action="store_true",
                   help="also call set_instrument_status (sets LAB_ALLOW_WRITES=1)")
    p.set_defaults(func=cmd_smoke)

    p = sub.add_parser("run", help="start the server over stdio from .venv")
    p.add_argument("--allow-writes", action="store_true", help="set LAB_ALLOW_WRITES=1")
    p.set_defaults(func=cmd_run)

    p = sub.add_parser("codex-config", help="print the Codex block with this machine's paths")
    p.set_defaults(func=cmd_codex_config)

    args = parser.parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
