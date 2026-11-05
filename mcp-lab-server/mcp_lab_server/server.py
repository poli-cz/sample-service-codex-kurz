"""MCP server over stdio with three tools on simulated lab instrument data.

Tools:
  list_instruments       read only
  get_results            read only, control (QC) results with lot targets
  set_instrument_status  write; refused unless the server runs with LAB_ALLOW_WRITES=1

The read-only tools carry the MCP annotation readOnlyHint, so a client can treat the write
tool differently (Codex: default_tools_approval_mode = "writes").
"""

from __future__ import annotations

import inspect
from typing import Any, Callable, Dict, List, Optional, TypeVar

from mcp.server.mcpserver import MCPServer
from mcp.server.mcpserver.exceptions import ToolError
from mcp_types import ToolAnnotations

from .store import LabStore, Settings, StoreError

READ_ONLY = ToolAnnotations(read_only_hint=True, open_world_hint=False)
WRITE = ToolAnnotations(
    read_only_hint=False, destructive_hint=False, idempotent_hint=True, open_world_hint=False
)


F = TypeVar("F", bound=Callable[..., Any])


def _tool(server: MCPServer, annotations: ToolAnnotations) -> Callable[[F], F]:
    """Registers a tool with its docstring, dedented, as the description for the model."""

    def register(fn: F) -> F:
        return server.tool(annotations=annotations, description=inspect.cleandoc(fn.__doc__ or ""))(fn)

    return register


def create_server(store: Optional[LabStore] = None) -> MCPServer:
    store = store or LabStore(Settings.from_env())
    server = MCPServer(
        "lab",
        instructions=(
            "Simulated laboratory instruments and their control (QC) results. "
            "All data is invented for a training course."
        ),
    )

    @_tool(server, READ_ONLY)
    def list_instruments(status: Optional[str] = None) -> List[Dict[str, Any]]:
        """List lab instruments with id, name, model, location, status and assays.

        status: optional filter, one of online, maintenance, out_of_service.
        """
        try:
            return store.list_instruments(status)
        except StoreError as e:
            raise ToolError(str(e)) from e

    @_tool(server, READ_ONLY)
    def get_results(instrument_id: str, assay: Optional[str] = None, limit: int = 20) -> Dict[str, Any]:
        """Control (QC) results of one instrument, oldest first.

        Every result has the control lot, its target mean and target SD, the time and the
        measured value. instrument_id looks like AN-01; assay is an optional code like GLU;
        limit (1 to 100) keeps only the newest results.
        """
        try:
            return store.get_results(instrument_id, assay, limit)
        except StoreError as e:
            raise ToolError(str(e)) from e

    @_tool(server, WRITE)
    def set_instrument_status(instrument_id: str, status: str, reason: str) -> Dict[str, Any]:
        """Change the status of an instrument (online, maintenance, out_of_service).

        The change and its reason go to a change log. The server refuses
        the call unless it was started with LAB_ALLOW_WRITES=1.
        """
        try:
            return store.set_instrument_status(instrument_id, status, reason)
        except StoreError as e:
            raise ToolError(str(e)) from e

    return server


def main() -> None:
    create_server().run("stdio")
