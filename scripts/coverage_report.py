#!/usr/bin/env python3
"""Print line coverage per first-party source file after the coverage preset ran.

Example:
    python scripts/check_all.py coverage
    scripts/coverage_report.py              # uses build/coverage
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

LINE_RE = re.compile(r"File '([^']+)'\nLines executed:([0-9.]+)% of (\d+)")


def collect(build_dir: Path, source_dir: Path, gcov: str) -> dict[str, tuple[float, int]]:
    results: dict[str, tuple[float, int]] = {}
    for gcda in sorted(build_dir.rglob("*.gcda")):
        if "/tests/" in gcda.as_posix():
            continue
        out = subprocess.run([gcov, "-n", "-o", str(gcda.parent), str(gcda)],
                             capture_output=True, text=True, check=False).stdout
        for path, percent, lines in LINE_RE.findall(out):
            source = Path(path)
            if source.suffix != ".cpp" or not source.is_relative_to(source_dir):
                continue
            rel = source.relative_to(source_dir).as_posix()
            if rel.startswith(("src/", "apps/")):
                results[rel] = (float(percent), int(lines))
    return results


def main(argv: list[str] | None = None) -> int:
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build-dir", type=Path, default=root / "build" / "coverage")
    parser.add_argument("--gcov", default="gcov")
    args = parser.parse_args(argv)
    if not args.build_dir.is_dir():
        parser.error(f"{args.build_dir} does not exist, run: python scripts/check_all.py coverage")
    results = collect(args.build_dir, root, args.gcov)
    if not results:
        print("no coverage data found", file=sys.stderr)
        return 1
    for rel in sorted(results):
        percent, lines = results[rel]
        print(f"{rel:<50} {percent:6.1f}% of {lines:4d} lines")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
