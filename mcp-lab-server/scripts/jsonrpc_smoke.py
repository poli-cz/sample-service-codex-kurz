#!/usr/bin/env python3
"""Talks to the server over stdio with raw JSON-RPC, without any MCP client library.

    python scripts/jsonrpc_smoke.py            # uses the current interpreter
    LAB_ALLOW_WRITES=1 python scripts/jsonrpc_smoke.py --write

Prints every request and response, one JSON object per line. Useful to see what an MCP
client such as Codex actually sends, and to check the server without Codex.
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent


def main() -> int:
    write = "--write" in sys.argv
    proc = subprocess.Popen(
        [sys.executable, "-m", "mcp_lab_server"],
        cwd=REPO,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=sys.stderr,
        text=True,
    )
    assert proc.stdin and proc.stdout
    next_id = 0

    def send(method: str, params: dict | None = None, notify: bool = False) -> dict | None:
        nonlocal next_id
        msg: dict = {"jsonrpc": "2.0", "method": method}
        if params is not None:
            msg["params"] = params
        if not notify:
            next_id += 1
            msg["id"] = next_id
        print(">>", json.dumps(msg))
        proc.stdin.write(json.dumps(msg) + "\n")
        proc.stdin.flush()
        if notify:
            return None
        line = proc.stdout.readline()
        print("<<", line.strip())
        return json.loads(line)

    send("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                        "clientInfo": {"name": "jsonrpc_smoke", "version": "0"}})
    send("notifications/initialized", notify=True)
    tools = send("tools/list")
    names = [t["name"] for t in tools["result"]["tools"]]
    print("tools:", names)
    send("tools/call", {"name": "list_instruments", "arguments": {}})
    send("tools/call", {"name": "get_results",
                        "arguments": {"instrument_id": "AN-02", "assay": "GLU", "limit": 3}})
    send("tools/call", {"name": "get_results", "arguments": {"instrument_id": "../etc"}})
    if write:
        send("tools/call", {"name": "set_instrument_status",
                            "arguments": {"instrument_id": "AN-02", "status": "maintenance",
                                          "reason": "QC 2-2s on GLU"}})
    proc.stdin.close()
    return proc.wait(timeout=10)


if __name__ == "__main__":
    sys.exit(main())
