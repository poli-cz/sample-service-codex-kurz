import os
from pathlib import Path
from mcp.server.mcpserver import MCPServer
from mcp.server.mcpserver.exceptions import ToolError
from mcp_types import ToolAnnotations

TICKETS = Path(os.environ.get("TICKETS_DIR", "tickets")).resolve()
server = MCPServer("tickets", instructions="Team tickets. Read only.")
READ_ONLY = ToolAnnotations(read_only_hint=True)

@server.tool(annotations=READ_ONLY)
def list_tickets() -> list[str]:
    """List the ids of all tickets, like LAB-401, sorted."""
    return sorted(p.stem for p in TICKETS.glob("*.md"))

@server.tool(annotations=READ_ONLY)
def get_ticket(ticket_id: str) -> str:
    """Full Markdown text of one ticket. ticket_id looks like LAB-401."""
    path = (TICKETS / f"{ticket_id}.md").resolve()
    if path.parent != TICKETS or not path.is_file():
        raise ToolError(f"unknown ticket {ticket_id}")
    return path.read_text(encoding="utf-8")

if __name__ == "__main__":
    server.run("stdio")
