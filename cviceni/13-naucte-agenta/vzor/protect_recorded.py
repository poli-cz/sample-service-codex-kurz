"""PreToolUse hook: captures from real analyzers in tests/data/recorded are read-only.

Codex sends the planned tool call as JSON on stdin. Exit 0 lets it run, exit 2 blocks it
and Codex gives the text on stderr to the agent as the reason.
"""

import json
import re
import sys

RECORDED = r"tests[/\\]+data[/\\]+recorded"
SHELL_WRITE = r"(>|\btee\b|\bsed\s+-i|\brm\b|\bmv\b|\bcp\b|Set-Content|Add-Content|Out-File|Remove-Item|Move-Item|Copy-Item)"

call = json.load(sys.stdin)
tool = call.get("tool_name", "")
command = str(call.get("tool_input", {}).get("command", ""))

if tool == "apply_patch":
    blocked = re.search(r"\*\*\* (Add|Update|Delete) File: [^\n]*" + RECORDED, command)
else:
    blocked = re.search(RECORDED, command) and re.search(SHELL_WRITE, command)

if blocked:
    print("tests/data/recorded/ holds captures from real analyzers and is read-only. "
          "Put new test cases into unit tests in tests/*_test.cpp. If a capture no longer "
          "decodes as recorded, your change broke compatibility: fix the code, not the capture.",
          file=sys.stderr)
    sys.exit(2)
sys.exit(0)
