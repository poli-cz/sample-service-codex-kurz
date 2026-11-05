# mcp-lab-server

A small MCP server over stdio with simulated laboratory instruments and their control (QC)
results. It is the sample server of the AICODX course: you finish it, connect it to Codex and
decide which of its tools the agent may use. All data is invented.

| tool | kind | what it does |
|---|---|---|
| `list_instruments` | read only | instruments with id, name, model, location, status and assays; optional status filter |
| `get_results` | read only | control results of one instrument (optionally one assay), oldest first, with lot, target mean and target SD |
| `set_instrument_status` | write | changes the status of an instrument; refused unless the server runs with `LAB_ALLOW_WRITES=1` |

The read-only tools carry the MCP annotation `readOnlyHint`, the write tool does not.

## Requirements

- Python 3.10 or newer. The official MCP Python SDK (`mcp`) requires 3.10 in every release
  since 1.0, so Python 3.9 is not enough.
- Network access to PyPI once, to install the pinned packages.

## Set up and test

`scripts/dev.py` (standard library only) does the same on every operating system and shell. Run it
with `python` or `python3`, whichever starts Python 3.10 or newer:

```sh
python scripts/dev.py setup          # creates .venv, installs requirements-dev.txt
python scripts/dev.py test           # pytest inside .venv
```

Without Codex, talk to the server with raw JSON-RPC over stdio:

```sh
python scripts/dev.py smoke
python scripts/dev.py smoke --write  # also calls set_instrument_status, with LAB_ALLOW_WRITES=1
```

`python scripts/dev.py run [--allow-writes]` starts the server itself over stdio. There is no need
to activate `.venv`: the script finds its interpreter.

## Configuration

| variable | default | meaning |
|---|---|---|
| `LAB_DATA_DIR` | `mcp_lab_server/data` | directory with `instruments.json` and `qc_results.json` |
| `LAB_STATE_DIR` | `./var` | where status changes are logged (`status_changes.jsonl`) |
| `LAB_ALLOW_WRITES` | unset | `1` allows `set_instrument_status`; anything else refuses it |

## Connect to Codex

`python scripts/dev.py codex-config` prints the complete block with the absolute paths of this
checkout and the path of your Codex `config.toml`; paste it there. `codex/config.example.toml` has
the same block with placeholders. In short:

```toml
[mcp_servers.lab]
command = "/ABS/PATH/mcp-lab-server/.venv/bin/python"
args = ["-m", "mcp_lab_server"]
cwd = "/ABS/PATH/mcp-lab-server"
env = { LAB_STATE_DIR = "/ABS/PATH/mcp-lab-server/var" }
enabled_tools = ["list_instruments", "get_results"]
default_tools_approval_mode = "writes"
```

Keep everything in this one table: `env` as an inline table, a per-tool approval as a dotted key
(`tools.set_instrument_status.approval_mode = "prompt"`). Codex adds its own tables to
`config.toml`, sometimes right after this one, so a separate `[mcp_servers.lab.env]` table does not
stay last, and keys added below the table later belong to the wrong table. Add new keys of this
server inside the table, next to the others.

`codex mcp add lab --env LAB_STATE_DIR=... -- <python in .venv> -m mcp_lab_server`
writes `command`, `args` and the `env` table, but no `cwd`: add `cwd` by hand, otherwise
Python cannot find the package. Check with
`codex mcp list`, `codex mcp get lab`, and `/mcp` inside a Codex session.

## Security notes

- Least privilege on two levels: the client (`enabled_tools`, approval mode) decides what the
  agent sees, the server (`LAB_ALLOW_WRITES`) decides what it executes. Keep both.
- Every argument is validated against a fixed pattern or list; nothing from a tool call ends up
  in a file path, a shell command or a query.
- A real server for GitLab, Jira or a LIMS needs a token: give it the smallest scope that
  works (read only, one project), pass it through the environment (`env_vars` in the Codex
  configuration, or `bearer_token_env_var` for HTTP servers) and never write it into a
  `config.toml` that is committed.
- Tool output is input for the agent like any other text and can carry a prompt injection.

## Layout

```
mcp_lab_server/server.py   MCP layer: tools, annotations, stdio entry point
mcp_lab_server/store.py    data access and validation, plain Python
mcp_lab_server/data/       invented instruments and control results
tests/                     pytest: store, MCP in process, MCP over stdio
scripts/jsonrpc_smoke.py   raw JSON-RPC client for manual checks
scripts/dev.py             setup, test, smoke, run, codex-config on any platform
codex/config.example.toml  Codex configuration
```
