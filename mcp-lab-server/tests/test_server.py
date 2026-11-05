"""The MCP layer: tool list, annotations and calls, in process and over stdio."""

from __future__ import annotations

import json
import os
import sys
from pathlib import Path

import anyio
from mcp import Client
from mcp.client.stdio import StdioServerParameters

from mcp_lab_server.server import create_server
from mcp_lab_server.store import PACKAGE_DATA, LabStore, Settings

REPO = Path(__file__).resolve().parent.parent


def test_tools_and_annotations(tmp_path: Path) -> None:
    server = create_server(LabStore(Settings(PACKAGE_DATA, tmp_path, allow_writes=False)))

    async def run() -> dict:
        async with Client(server) as client:
            listed = await client.list_tools()
            return {t.name: t.annotations for t in listed.tools}

    tools = anyio.run(run)
    assert set(tools) == {"list_instruments", "get_results", "set_instrument_status"}
    assert tools["list_instruments"].read_only_hint is True
    assert tools["get_results"].read_only_hint is True
    assert tools["set_instrument_status"].read_only_hint is False


def test_write_tool_reports_an_error_when_writes_are_off(tmp_path: Path) -> None:
    server = create_server(LabStore(Settings(PACKAGE_DATA, tmp_path, allow_writes=False)))

    async def run():
        async with Client(server) as client:
            return await client.call_tool(
                "set_instrument_status",
                {"instrument_id": "AN-02", "status": "maintenance", "reason": "QC failure"},
            )

    result = anyio.run(run)
    assert result.is_error
    assert "disabled" in result.content[0].text


def test_stdio_round_trip(tmp_path: Path) -> None:
    """Starts `python -m mcp_lab_server` exactly as Codex does and calls a tool."""
    params = StdioServerParameters(
        command=sys.executable,
        args=["-m", "mcp_lab_server"],
        cwd=str(REPO),
        env={"PATH": os.environ.get("PATH", ""), "LAB_STATE_DIR": str(tmp_path)},
    )

    async def run():
        async with Client(params) as client:
            return await client.call_tool("get_results", {"instrument_id": "AN-02", "limit": 2})

    result = anyio.run(run)
    assert not result.is_error
    payload = json.loads(result.content[0].text)
    assert [(r["assay"], r["value"]) for r in payload["results"]] == [("GLU", 5.98), ("CRP", 50.3)]
